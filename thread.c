/* File: curl.c
 * Creation Date: March 20th, 2010
 * Last Modified Date: March 20th, 2010
 * Version: 0.0.1
 * Contact: Adam Lamers <adam@millenniumsoftworks.com>
*/

#include <curl/curl.h>
#include <curl/easy.h>
#include <file_get_contents.h>
#include "version.h"

/**
  * Function to post a reply to a 4chan thread.
  * @param postlocation The http POST recieve location (ex: http://sys.4chan.org/b/imgboard.php)
  * @param threadno The thread number to reply to.
  * @param imagePath The location of the image to attach to the reply.
  * @param name Name field in the post.
  * @param email Email field in the post.
  * @param sub Subject field in the post.
  * @param com Body field in the post.
  * @param pwd Password to delete the post.
  */
int chan_threadreply(char *postlocation, char *threadno, char *imagePath, char *name, char *email, char *sub, char *com, char *pwd)
{
    CURL *curl;
    CURLcode res = CURLE_AGAIN;
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "MAX_FILE_SIZE", CURLFORM_COPYCONTENTS, "99999999", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "resto", CURLFORM_COPYCONTENTS, threadno, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "name", CURLFORM_COPYCONTENTS, name, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "email", CURLFORM_COPYCONTENTS, email, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "sub", CURLFORM_COPYCONTENTS, sub, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "com", CURLFORM_COPYCONTENTS, com, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "pwd", CURLFORM_COPYCONTENTS, pwd, CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "mode", CURLFORM_COPYCONTENTS, "regist", CURLFORM_END);
    if(imagePath != NULL)
    {
        void *imageData;
        long imageLength;
        if(file_get_contents(imagePath, &imageData, &imageLength) != 0) return CURLE_FILE_COULDNT_READ_FILE;
        curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "upfile",
                                          CURLFORM_BUFFER, imagePath,
                                          CURLFORM_BUFFERPTR, imageData,
                                          CURLFORM_BUFFERLENGTH, imageLength,
                                          CURLFORM_END);
    }
    
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, VERSION_FULLSTRING);
        curl_easy_setopt(curl, CURLOPT_URL, postlocation);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
    }
    return res;
}
