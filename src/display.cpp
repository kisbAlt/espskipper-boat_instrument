#include <Arduino.h>
#include <string.h>
#include <display.h>

#include <Preferences.h>

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

Preferences preferences;
const StringTranslations eng_strings = {"AVG", "COURSE", "MAX", "DISTANCE", "KM/H", "KNOTS", "Satellites", "TEMP", "DEPTH"};
const StringTranslations hu_strings = {"ATLAG", "IRANY", "MAXIMUM", "TAV", "KM/H", "CSOMO", "muhold", "HOFOK", "MELYSEG"};



DisplayHandler::DisplayHandler() : display1(U8G2_R0, /* clk=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* reset=*/ 22), display2(U8G2_R0, /* clk=*/ 19, /* data=*/ 21, /* cs=*/ 22, /* reset=*/ 22)
{
    Serial.println("Constructor");
}

void DisplayHandler::Init()
{
    dispSettings.LoadData();
    display1.begin();

    display2.begin();
}

void DisplayHandler::PrepareDraw() {
    display1.clearBuffer();
    display1.setFont(u8g2_font_6x10_tf);
    display1.setFontRefHeightExtendedText();
    display1.setDrawColor(1);
    display1.setFontPosTop();
    display1.setFontDirection(0);

    display2.clearBuffer();
    display2.setFont(u8g2_font_6x10_tf);
    display2.setFontRefHeightExtendedText();
    display2.setDrawColor(1);
    display2.setFontPosTop();
    display2.setFontDirection(0);
}

char maxBuf[10];
char avgBuf[10];
char distBuf[10];
char courseBuf[4];
char depthBuf[4];
char tempBuf[4];
void DisplayHandler::DrawUI(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings)
{
    StringTranslations texts = getLangTranslations();
    if (!prepared) {
        PrepareDraw(); 
        prepared = true;
    }

    display2.clearBuffer();
    display2.setFont(u8g2_font_5x7_tf);
    DrawUIBox();
    display2.drawStr(0,0, texts.AvgSpeed);
    display2.drawStr(44,0, texts.Course);
    display2.drawStr(86,0, texts.MaxSpeed);

    display2.drawStr(0,32, texts.Distance);
    display2.drawStr(44,32, texts.WaterTemp);
    display2.drawStr(86,32, texts.Depth);

    display2.setFont(u8g2_font_logisoso16_tn);

    // Writing stat datas to buffers
    dtostrf(stats.maxSpeedKmph, 3, 1, maxBuf);
    dtostrf(stats.avgSpeedKmph, 3, 1, avgBuf);
    dtostrf(stats.distance, 3, 1, distBuf);
    dtostrf(0, 3, 1, tempBuf);
    dtostrf(0, 3, 1, depthBuf);
    
    snprintf(courseBuf, sizeof(courseBuf), "%03d", stats.lastCourse);


    // drawing buffers to screen
    display2.drawStr(0,10, avgBuf);
    display2.drawStr(44,10, courseBuf);
    display2.drawStr(86,10, maxBuf);

    display2.drawStr(0,42, distBuf);
    display2.drawStr(44,42, tempBuf);
    display2.drawStr(86,42, depthBuf);

    display2.sendBuffer();

    // reading ESP32 CPU temp
    int temp_celsius = round(temperatureRead());
    sprintf(satsBuf, "S%d 192.168.4.1 %d/%d %dC", satellites, dispSettings.fullRefreshTime/1000, dispSettings.speedRefreshTime/1000, temp_celsius);

}

void DisplayHandler::DrawUIBox()
{
    display2.drawLine(42, 0, 42, 64);
    display2.drawLine(84, 0, 84, 64);

    display2.drawLine(0, 31, 128, 31);
}

void DisplayHandler::DrawSmallText(char text[], int16_t x, int16_t y, bool centerX)
{
}

void DisplayHandler::DrawMediumText(char text[], int16_t x, int16_t y, bool centerX)
{
}

void DisplayHandler::DrawLargeText(char text[], int16_t x, int16_t y, bool centerX)
{
}

void DisplayHandler::DrawSpeed(double speed)
{
    char buffer[10];
    // itoa(speed, buffer, 10); // 10 = base (decimal)
    if (speed >= 10) {
        dtostrf(speed, 3, 1, buffer);
    }else {
        dtostrf(speed, 3, 2, buffer);
    }

    if (buffer == lastBuffer) {
        return;
    }
    //lastBuffer = buffer;

    const char* speedFormat;
    if (dispSettings.useKnots) {
        speedFormat = getLangTranslations().Knots;

    }else {
        speedFormat = getLangTranslations().Kmph;
    }

    int textWidth = display1.getStrWidth(speedFormat);
    int x = (128 - textWidth) / 2;

    display1.clearBuffer();
    display1.setFont(u8g2_font_spleen32x64_mn);
    display1.drawStr(0, 0,buffer);
    display1.setFont(u8g2_font_5x7_tf);
    display1.drawStr(0,56, satsBuf);
    display1.drawStr(x, 0, speedFormat);
    display1.sendBuffer();
}

void DisplayHandler::PartialCourse(u_int16_t course)
{
}


void DisplayHandler::DrawDisplay1() {

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