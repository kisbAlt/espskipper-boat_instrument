#include <Arduino.h>
#include <string.h>
#include <display.h>

#include <Preferences.h>

#include <U8g2lib.h>

#include <accelerometerhandler.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define MOSFET_PIN 4

Preferences preferences;
const static StringTranslations eng_strings = {"AVG", "COURSE", "MAX", "DISTANCE", "KM/H", "KNOTS", "Satellites", "TEMP", "DEPTH", "Kilometers", "Celsius", "Meters", "Degrees", "Interval", "ROLL"};
const static StringTranslations hu_strings = {"ÁTLAG", "IRÁNY", "MAXIMUM", "TÁVOLSÁG", "KM/H", "CSOMÓ", "muhold", "HÕFOK", "MÉLYSÉG", "Kilóméter", "Celsius", "Méter", "Fok", "Idokoz", "DÕLÉS"};
const uint8_t TIMEZONE_SHIFT = 2;
AccelerometerHandler accelerometer;
const int MAX_GYRO_DRAWHEIGHT = 55;

DisplayHandler::DisplayHandler() : display1(U8G2_R0, /* clk=*/18, /* data=*/23, /* cs=*/5, /* reset=*/22), display2(U8G2_R0, /* clk=*/19, /* data=*/21, /* cs=*/22, /* reset=*/22)
{
}

void DisplayHandler::Init()
{
    dispSettings.LoadData();
    display1.begin();
    display1.enableUTF8Print();

    display2.begin();
    display2.enableUTF8Print();
    PrepareDraw();
    pinMode(MOSFET_PIN, OUTPUT);

    accelerometer.Init();
}

void DisplayHandler::PrepareDraw()
{
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
char depthBuf[6];
char tempBuf[4];
void DisplayHandler::DrawDisplay2(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings, u_int16_t depth)
{
    StringTranslations texts = getLangTranslations();

    const char *speedUnitText = GetSpeedUnitText();
    dtostrf(stats.GetMaxSpeed(displaySettings.useKnots), 3, 1, maxBuf);
    dtostrf(stats.GetAvgSpeed(displaySettings.useKnots), 3, 1, avgBuf);

    dtostrf(stats.distance, 3, 1, distBuf);
    dtostrf(0, 3, 1, tempBuf);

    if (depth > 0) {
        Serial.println("depthdraw");
        Serial.println(depth);
        float depth_offset = (roundf((depth+displaySettings.depthOffset)/10.0f))/10.0f;
        Serial.println(depth_offset);
        dtostrf(depth_offset, 4, 1, depthBuf);
    }


    snprintf(courseBuf, sizeof(courseBuf), "%03d", stats.lastCourse);

    // Needing a seperate if statement, because the display doesen't need to be updated at every execution
    if (display2State == SPEED_HISTORY)
    {
        if (speedHistoryUpdated)
        {
            display2.clearBuffer();
            speedHistoryUpdated = false;
            DrawSpeedHistory();
            display2.sendBuffer();
        }
    }
    else
    {

        display2.clearBuffer();

        if (display2State == SUMMARY)
        {
            DrawSummary();
        }
        else if (display2State == GYRO)
        {
            DrawGyro();
        }
        else
        {
            char *titleText;
            const char *unitText;
            char *drawBuf;
            if (display2State == COURSE)
            {
                titleText = texts.Course;
                drawBuf = courseBuf;
                unitText = texts.Degrees;
            }
            else if (display2State == DEPTH)
            {
                titleText = texts.Depth;
                drawBuf = depthBuf;
                unitText = texts.Meters;
            }
            else if (display2State == TEMP)
            {
                titleText = texts.WaterTemp;
                drawBuf = tempBuf;
                unitText = texts.Celsius;
            }
            else if (display2State == SPEED_AVG)
            {
                titleText = texts.AvgSpeed;
                drawBuf = avgBuf;
                unitText = speedUnitText;
            }
            else if (display2State == SPEED_MAX)
            {
                titleText = texts.MaxSpeed;
                drawBuf = maxBuf;
                unitText = speedUnitText;
            }
            else if (display2State == DISTANCE)
            {
                titleText = texts.Distance;
                drawBuf = distBuf;
                unitText = texts.Kilometers;
            }

            display2.setFont(u8g2_font_spleen32x64_mn);
            int textWidth = display2.getStrWidth(drawBuf);
            int x = (128 - textWidth) / 2;
            display2.drawStr(x, -1, drawBuf);

            display2.setFont(u8g2_font_6x12_t_symbols);

            textWidth = display2.getStrWidth(titleText);
            x = (128 - textWidth) / 2;
            display2.drawUTF8(x, 54, titleText);

            textWidth = display2.getStrWidth(unitText);
            x = (128 - textWidth) / 2;

            display2.drawUTF8(x, 0, unitText);
        }
        display2.sendBuffer();
    }
}

void DisplayHandler::DrawSummary()
{
    StringTranslations texts = getLangTranslations();
    display2.setFont(u8g2_font_5x7_tf);
    DrawUIBox();
    display2.drawUTF8(0, 0, texts.AvgSpeed);
    display2.drawUTF8(44, 0, texts.Course);
    display2.drawUTF8(86, 0, texts.MaxSpeed);

    display2.drawUTF8(0, 32, texts.Distance);
    display2.drawUTF8(44, 32, texts.WaterTemp);
    display2.drawUTF8(86, 32, texts.Depth);

    display2.setFont(u8g2_font_logisoso16_tn);

    // drawing buffers to screen
    display2.drawStr(0, 10, avgBuf);
    display2.drawStr(44, 10, courseBuf);
    display2.drawStr(86, 10, maxBuf);

    display2.drawStr(0, 42, distBuf);
    display2.drawStr(44, 42, tempBuf);
    display2.drawStr(86, 42, depthBuf);
}

void DisplayHandler::DrawGyro()
{
    accelerometer.UpdateGyro();

    StringTranslations texts = getLangTranslations();
    display2.setFont(u8g2_font_6x12_t_symbols);

    int textWidth = display2.getStrWidth(texts.Roll);
    int x = (128 - textWidth) / 2;
    display2.drawUTF8(x, 0, texts.Roll);

    int rolldisplay = 180 - (int)round(fabs(accelerometer.lastRoll));
    display2.setFont(u8g2_font_9x15_t_symbols);
    sprintf(rollBuf, "%d°", rolldisplay);
    display2.drawUTF8(0, 0, rollBuf);

    float angleRad = rolldisplay * PI / 180.0; // Convert to radians
    int opposite = (int)(tan(angleRad) * 127);
    if (opposite <= MAX_GYRO_DRAWHEIGHT)
    {
        if (accelerometer.lastRoll > 0)
        {
            display2.drawTriangle(0, 63, 127, 63, 127, 63 - opposite);
        }
        else
        {
            display2.drawTriangle(0, 63, 127, 63, 0, 63 - opposite);
        }
    }
    else
    {
        int adjacent = (int)(MAX_GYRO_DRAWHEIGHT / tan(angleRad)); // compute adjacent
        if (accelerometer.lastRoll > 0)
        {
            display2.drawTriangle(0, 63, adjacent, 63, adjacent, 63 - MAX_GYRO_DRAWHEIGHT);
            display2.drawBox(adjacent,63-MAX_GYRO_DRAWHEIGHT,127-adjacent,63);
        }
        else
        {
            int x2 =  127-adjacent;
            display2.drawTriangle(x2, 63, 127, 63, x2, 63 - MAX_GYRO_DRAWHEIGHT);
            display2.drawBox(0,63-MAX_GYRO_DRAWHEIGHT,127-adjacent,63);
        }
    }

    // display2.drawTriangle();
}

void DisplayHandler::DrawUIBox()
{
    // Draw horizontal lines
    display2.drawLine(42, 0, 42, 64);
    display2.drawLine(84, 0, 84, 64);

    // Draw vertical line on the middle
    display2.drawLine(0, 31, 128, 31);
}

void DisplayHandler::DrawDisplay1(BoatStats stats, u_int32_t satellites, unsigned long now)
{

    double speed = stats.GetLastSpeed(dispSettings.useKnots);

    const char *speedFormat = GetSpeedUnitText();

    // If speed is less then two digits draw at 2 decimal
    if (speed >= 10)
    {
        dtostrf(speed, 3, 1, speedBuffer);
    }
    else
    {
        dtostrf(speed, 3, 2, speedBuffer);
    }

    // reading ESP32 CPU temp
    cpu_temp_celsius = round(temperatureRead());
    sprintf(satsBuf, "S%d 192.168.4.1 %d/%d %dC", satellites, dispSettings.fullRefreshTime / 1000, dispSettings.speedRefreshTime / 1000, cpu_temp_celsius);

    sprintf(timeBuf, "%02d:%02d", (stats.lastHour + TIMEZONE_SHIFT) % 24, stats.lastMinute);

    display1.clearBuffer();

    display1.setFont(u8g2_font_spleen32x64_mn);
    display1.drawStr(0, 0, speedBuffer);
    display1.setFont(u8g2_font_5x7_tf);
    display1.drawStr(0, 56, satsBuf);
    display1.drawStr(0, 0, timeBuf);
    display1.setFont(u8g2_font_6x12_t_symbols);
    int textWidth = display1.getStrWidth(speedFormat);
    int x = (128 - textWidth) / 2;
    display1.drawUTF8(x, 0, speedFormat);

    if (now - lastHistoryUpdate >= dispSettings.speedGraphUpdate)
    {
        // Save KMPH data to history
        SaveSpeedHistory(stats.GetLastSpeed(false));
        speedHistoryUpdated = true;
        numOfHistoryAvgCount = 2;
        currentHistoryAvg = stats.GetLastSpeed(false);
        lastHistoryUpdate = now;
    }
    else
    {
        currentHistoryAvg = currentHistoryAvg + (stats.GetLastSpeed(false) - currentHistoryAvg) / numOfHistoryAvgCount;
        numOfHistoryAvgCount++;
    }

    display1.sendBuffer();
}

const StringTranslations &DisplayHandler::getLangTranslations()
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

// Click counts, -1 value is long press
void DisplayHandler::HandleButtonInput(int clickCount)
{
    if (clickCount == 1)
    {
        // If single click detected increase the enum value of display2State by one
        display2State = static_cast<DisplayState>((static_cast<int>(display2State) + 1) % (maxDisplayState + 1));
    }
    else if (clickCount == 2)
    {
        if (dispSettings.useKnots)
        {
            dispSettings.useKnots = false;
        }
        else
        {
            dispSettings.useKnots = true;
        }
        dispSettings.SaveData();
    }
    else if (clickCount == -1)
    {
        if (dispSettings.backlight_on)
        {
            digitalWrite(MOSFET_PIN, LOW);
            dispSettings.backlight_on = false;
        }
        else
        {
            digitalWrite(MOSFET_PIN, HIGH);
            dispSettings.backlight_on = true;
        }
        dispSettings.SaveData();
    }
}

void DisplaySettings::SaveData()
{
    // saving settings presistently on the ESP32

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

const char *DisplayHandler::GetSpeedUnitText()
{
    if (dispSettings.useKnots)
    {
        return getLangTranslations().Knots;
    }
    else
    {
        return getLangTranslations().Kmph;
    }
}

void DisplayHandler::SaveSpeedHistory(double speed)
{
    if (historyIndex < SPEED_HISTORY_COUNT)
    {
        speedHistory[historyIndex] = speed;
        historyIndex = historyIndex + 1;
    }
    else
    {
        // Shift all values left by one
        for (int i = 0; i < SPEED_HISTORY_COUNT; ++i)
        {
            speedHistory[i] = speedHistory[i + 1];
        }

        // Add the new value at the end
        speedHistory[SPEED_HISTORY_COUNT - 1] = speed;
    }
}

void DisplayHandler::DrawSpeedHistory()
{
    // Finding max speed value
    // Saved only in Kmph, to display we have to convert to knots if needed
    double maxVal = 1;
    for (int i = 1; i < SPEED_HISTORY_COUNT; ++i)
    {
        if (speedHistory[i] > maxVal)
        {
            maxVal = speedHistory[i];
        }
    }
    for (int x = 0; x < SPEED_HISTORY_COUNT; ++x)
    {
        int y1 = (speedHistory[x] / maxVal) * SPEED_GRAPH_HEIGHT;
        display2.drawLine(x, 63, x, 63 - y1);
        Serial.println(y1);
    }
    char histBuf[40];
    sprintf(histBuf, "M:%d, %ds", maxVal, dispSettings.speedGraphUpdate / 1000);

    display1.setFont(u8g2_font_5x7_tf);
    display1.drawStr(0, 0, histBuf);
}
