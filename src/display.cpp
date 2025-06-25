#include <Arduino.h>
#include <string.h>
#include <display.h>
#define ENABLE_GxEPD2_GFX 0

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Roboto_Medium_98.h>
#include <GxEPD2_BW.h>
#include <Preferences.h>

Preferences preferences;
const StringTranslations eng_strings = {"AVG speed", "COURSE", "MAX speed", "DISTANCE", "KM/H", "KNOTS", "Satellites", "TEMP", "DEPTH"};
const StringTranslations hu_strings = {"ATLAG", "IRANY", "MAXIMUM", "TAVOLSAG", "KM/H", "CSOMO", "muhold", "HOFOK", "MELYSEG"};

DisplayHandler::DisplayHandler() : display(GxEPD2_420_GDEY042T81(/*CS=5*/ 5, /*DC=*/0, /*RES=*/2, /*BUSY=*/15))
{
    Serial.println("Constructor");
}

void DisplayHandler::Init()
{
    dispSettings.LoadData();
    display.init(115200, true, 50, false);
}

char maxBuf[10];
char avgBuf[10];
char distBuf[10];
char courseBuf[4];
char depthBuf[4];
char tempBuf[4];
void DisplayHandler::DrawUI(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings)
{
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.firstPage();
    StringTranslations texts = getLangTranslations();
    do
    {
        dtostrf(stats.maxSpeedKmph, 3, 1, maxBuf);

        dtostrf(stats.avgSpeedKmph, 3, 1, avgBuf);

        dtostrf(stats.distance, 3, 1, distBuf);

        dtostrf(0, 3, 1, tempBuf);
        dtostrf(0, 3, 1, depthBuf);

        // itoa(stats.lastCourse, courseBuf, 10);
        snprintf(courseBuf, sizeof(courseBuf), "%03d", stats.lastCourse);

        display.fillScreen(GxEPD_WHITE);
        DrawUIBox();
        DrawSmallText(texts.AvgSpeed, 0, 0, false);
        DrawSmallText(texts.Course, 155, 0, false);
        DrawSmallText(texts.MaxSpeed, 0, 67, false);
        DrawSmallText(texts.Distance, 155, 67, false);

        DrawSmallText(texts.WaterTemp, 0, 134, false);
        DrawSmallText(texts.Depth, 155, 134, false);

        if (displaySettings.useKnots)
        {
            DrawMediumText(texts.Knots, 0, 195, true);
        }
        else
        {
            DrawMediumText(texts.Kmph, 0, 195, true);
        }

        DrawLargeText(avgBuf, 30, 20, false); // Drawing avg speed
        DrawLargeText(maxBuf, 30, 82, false); // drawing max speed

        DrawLargeText(courseBuf, 182, 20, false);
        DrawLargeText(distBuf, 182, 82, false); // Drawing distance

        DrawLargeText(tempBuf, 30, 149, false); // drawing water temp
        DrawLargeText(depthBuf, 182, 149, false); // Drawing depth

        char satsBuf[40];
        int temp_celsius = round(temperatureRead());
        sprintf(satsBuf, "S%d 192.168.4.1 %d/%d %dC", satellites, dispSettings.fullRefreshTime/1000, dispSettings.speedRefreshTime/1000, temp_celsius);
        DrawSmallText(satsBuf, 0, 382, false);
    } while (display.nextPage());
}

void DisplayHandler::DrawUIBox()
{
    display.drawLine(0, 130, display.width() - 1, 130, GxEPD_BLACK);
    display.drawLine(0, 131, display.width() - 1, 131, GxEPD_BLACK);
    display.drawLine(0, 129, display.width() - 1, 129, GxEPD_BLACK);

    display.drawLine(0, 65, display.width() - 1, 65, GxEPD_BLACK);
    display.drawLine(0, 66, display.width() - 1, 66, GxEPD_BLACK);
    display.drawLine(0, 64, display.width() - 1, 64, GxEPD_BLACK);

    display.drawLine(0, 195, display.width() - 1, 195, GxEPD_BLACK);
    display.drawLine(0, 196, display.width() - 1, 196, GxEPD_BLACK);
    display.drawLine(0, 194, display.width() - 1, 194, GxEPD_BLACK);

    display.drawLine(149, 0, 149, 194, GxEPD_BLACK);
    display.drawLine(150, 0, 150, 194, GxEPD_BLACK);
    display.drawLine(151, 0, 151, 194, GxEPD_BLACK);
}

void DisplayHandler::DrawSmallText(char text[], int16_t x, int16_t y, bool centerX)
{
    display.setFont(&FreeMonoBold9pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    if (centerX)
    {
        int16_t centerX = ((display.width() - tbw) / 2) - tbx;
        display.setCursor(centerX, y + tbh);
    }
    else
    {
        display.setCursor(x, y + tbh);
    }
    display.print(text);
}

void DisplayHandler::DrawMediumText(char text[], int16_t x, int16_t y, bool centerX)
{
    display.setFont(&FreeMonoBold18pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    if (centerX)
    {
        int16_t centerX = ((display.width() - tbw) / 2) - tbx;
        display.setCursor(centerX, y + tbh);
    }
    else
    {
        display.setCursor(x, y + tbh);
    }
    display.print(text);
}

void DisplayHandler::DrawLargeText(char text[], int16_t x, int16_t y, bool centerX)
{
    display.setFont(&FreeMonoBold24pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    if (centerX)
    {
        int16_t centerX = ((display.width() - tbw) / 2) - tbx;
        display.setCursor(centerX, y + tbh);
    }
    else
    {
        display.setCursor(x, y + tbh);
    }
    display.print(text);
}

void DisplayHandler::DrawSpeed(double speed)
{

    Serial.println("Printing speed");
    display.setRotation(1);
    display.setFont(&Roboto_Medium_98);
    display.setTextColor(GxEPD_BLACK);

    char buffer[10];
    // itoa(speed, buffer, 10); // 10 = base (decimal)
    dtostrf(speed, 3, 2, buffer);

    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = 320;

    display.setPartialWindow(23, y + tby, 277, tbh);
    display.firstPage();
    display.setCursor(x, y);
    do
    {
        display.print(buffer);
    } while (display.nextPage());
}

StringTranslations DisplayHandler::getLangTranslations()
{
    if (dispSettings.language == ENGLISH)
    {
        return eng_strings;
    }
    else if (dispSettings.language == HUNGARIAN)
    {
        return hu_strings;
    }
    else
    {
        return eng_strings;
    }
}

void DisplaySettings::SaveData()
{
    Serial.println("saving settings...");
    preferences.begin("disp", false);

    preferences.putBool("useKnots", useKnots);
    preferences.putUInt("refresh1", fullRefreshTime);
    preferences.putUInt("refresh2", speedRefreshTime);
    preferences.putInt("language", static_cast<int>(language));

    preferences.end();
}

void DisplaySettings::LoadData()
{
    Serial.println("loading settings...");
    preferences.begin("disp", false);

    if (preferences.isKey("useKnots"))
    {
        useKnots = preferences.getBool("useKnots", false);
    }
    if (preferences.isKey("refresh1"))
    {
        fullRefreshTime = preferences.getUInt("refresh1", false);
        Serial.println("fullrefresh: ");
        Serial.println(fullRefreshTime);
    }
    if (preferences.isKey("refresh2"))
    {
        speedRefreshTime = preferences.getUInt("refresh2", false);
    }
    if (preferences.isKey("language"))
    {
        language = static_cast<Language>(preferences.getInt("language", ENGLISH));
    }

    preferences.end();
}