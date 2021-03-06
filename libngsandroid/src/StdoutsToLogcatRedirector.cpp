#include "StdoutsToLogcatRedirector.h"

#include <iostream>
#include <unistd.h>

#include <android/log.h>


#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "NgsCore", fmt, ## __VA_ARGS__)

namespace ngs
{
// Redirect the "stdout" and "stderr" to android logcat.
// https://codelab.wordpress.com/2014/11/03/how-to-use-standard-output-streams-for-logging-in-android-apps/
// http://stackoverflow.com/a/31777050

    static int logcatPfd[2];

    static pthread_t stdoutsThread;

    static const char* pLogcatTag;


    static void* stdoutsThreadFunc(void*)
    {
        ssize_t rdsz;
        char buf[256];

        // workaround for android logcat formatting
        buf[0] = '-';
        buf[1] = 'N';
        buf[2] = 'G';
        buf[3] = 'S';
        buf[4] = '-';
        buf[5] = ' ';

        int logPrefixSize = 6;

        while ((rdsz = read(logcatPfd[0], buf + logPrefixSize, sizeof buf - 1 - logPrefixSize))
                > 0) {
//            if(buf[rdsz + 7 - 1 ] == '\n') --rdsz;
            buf[rdsz + logPrefixSize] = 0;      /* add null-terminator */
            __android_log_write(ANDROID_LOG_DEBUG, pLogcatTag, buf);
        }
        return (0);
    }


    int redirectStdoutsToLogcat(const char* pAppName)
    {
        pLogcatTag = pAppName;

        /* make stdout line-buffered and stderr unbuffered */
        setvbuf(stdout, 0, _IOLBF, 0);
        setvbuf(stderr, 0, _IONBF, 0);

        /* create the pipe and redirect stdout and stderr */
        pipe(logcatPfd);
        dup2(logcatPfd[1], 1);
        dup2(logcatPfd[1], 2);

        /* spawn the logging thread */
        if (pthread_create(&stdoutsThread, 0, stdoutsThreadFunc, 0) == -1) {
            return (-1);
        }

        pthread_detach(stdoutsThread);
        return (0);
    }


    bool StdoutsToLogcatRedirector::init()
    {
        LOG("-NGS- NgsLogger init %s", "starting");
        if (redirectStdoutsToLogcat("NgsCore")) {
            LOG("-NGS- NgsLogger init %s", "FAILED");
            return false;
        }
        LOG("-NGS- NgsLogger init %s", "finished");
        std::cout << "NgsLogger is ready" << std::endl;
        return true;
    }
}
