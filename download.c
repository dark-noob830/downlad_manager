#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void download_file(const char *url, const char *output_filename)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        FILE *file_o = fopen(output_filename, "wb");
        if (file_o)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_o);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
            }
            fclose(file_o);
        }
        else
        {
            fprintf(stderr, "Failed to open file %s for writing\n", output_filename);
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        fprintf(stderr, "error initializing curl\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <link_file> \n", argv[0]);
        return 1;
    }

    const char *link_file = argv[1];
    const char *output_dir = ".";

    FILE *file = fopen(link_file, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", link_file);
        return 1;
    }

    char url[1024];
    int file_number = 1;
    while (fgets(url, sizeof(url), file))
    {
        url[strcspn(url, "\n")] = 0;
        char *format_file = strrchr(url, '.');
        // printf("last : %s\n", format_file);
        char output_filename[1024];
        snprintf(output_filename, sizeof(output_filename), "%s/file_%d%s", output_dir, file_number, format_file);
        file_number++;
        pid_t pid = fork();
        if (pid == 0)
        {
            download_file(url, output_filename);
            // printf("download finished : %s\n",output_filename);
            exit(0);
        }
        else if (pid < 0)
        {
            fprintf(stderr, "fork error\n");
        }
    }
    int status;
    while (wait(&status) > 0)
        ;
    fclose(file);
    return 0;
}
