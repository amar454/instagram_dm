#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define MAX_LINE 1024

typedef struct {
    char username[50];
    char password[50];
    int followers;
    int following;
} InstagramAccount;

typedef struct {
    char name[100];
    char birthday[20];
    char instagram[50];
} Celebrity;

// Function to store received data from curl
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *data = (char *)userp;
    strcat(data, (char *)contents);
    return realsize;
}

// Function to scrape Famous Birthdays
Celebrity* get_celebrities(int *count) {
    CURL *curl;
    CURLcode res;
    char buffer[16384] = {0};
    Celebrity *celebrities = malloc(100 * sizeof(Celebrity));
    *count = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.famousbirthdays.com/today.html");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        
        res = curl_easy_perform(curl);
        
        if(res == CURLE_OK) {
            // Parse HTML and extract celebrity information
            // This is a simplified version - you'd need proper HTML parsing
            char *line = strtok(buffer, "\n");
            while(line != NULL && *count < 100) {
                // Basic parsing logic - you'd need more sophisticated parsing
                if(strstr(line, "class=\"name\"")) {
                    sscanf(line, "%*[^>]>%[^<]", celebrities[*count].name);
                    (*count)++;
                }
                line = strtok(NULL, "\n");
            }
        }
        curl_easy_cleanup(curl);
    }
    return celebrities;
}

// Function to check if Instagram account exists
int check_instagram(const char *username) {
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    
    curl = curl_easy_init();
    if(curl) {
        char url[100];
        snprintf(url, sizeof(url), "https://www.instagram.com/%s/", username);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        }
        curl_easy_cleanup(curl);
    }
    return (http_code == 200);
}

// Function to load bot accounts
InstagramAccount* load_bot_accounts(int *count) {
    FILE *file = fopen("bot_accounts.txt", "r");
    InstagramAccount *accounts = malloc(MAX_ACCOUNTS * sizeof(InstagramAccount));
    *count = 0;
    
    if(file) {
        char line[MAX_LINE];
        while(fgets(line, MAX_LINE, file) && *count < MAX_ACCOUNTS) {
            sscanf(line, "%[^,],%[^,],%d,%d", 
                   accounts[*count].username,
                   accounts[*count].password,
                   &accounts[*count].followers,
                   &accounts[*count].following);
            (*count)++;
        }
        fclose(file);
    }
    return accounts;
}

// Function to send Instagram DM
void send_instagram_dm(InstagramAccount *bot, const char *target, const char *message) {
    // This would need to use Instagram's API or a third-party library
    printf("Sending DM from %s to %s: %s\n", bot->username, target, message);
}

int main() {
    int celeb_count = 0;
    int *celebrities = get_celebrities(&celeb_count);
    
    // Load bot accounts
    InstagramAccount *bots = load_bot_accounts(&bot_count);
    
    if(bot_count == 0) {
        printf("No bot accounts found!\n");
        return 1;
    }
    
    // Process each celebrity
    for(int i = 0; i < celeb_count; i++) {
        if(check_instagram(celebrities[i].instagram)) {
            // Select random bot account
            int random_bot = rand() % bot_count;
            
            // Create personalized message
            char message[200];
            snprintf(message, sizeof(message), 
                    "Hey %s! Happy birthday! I'm a big fan of your work!", 
                    celebrities[i].name);
            
            // Send DM
            send_instagram_dm(&bots[random_bot], 
                            celebrities[i].instagram, 
                            message);
            
            // Wait random time to avoid detection
            sleep(rand() % 60 + 30);
        }
    }
    
    // Cleanup
    free(celebrities);
    free(bots);
    
    return 0;
}