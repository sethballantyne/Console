//  __            _                   
// / _|          | |                  
//| |_ ___  _ __ | |_ __ _  ___ _ __  
//|  _/ _ \| '_ \| __/ _` |/ _ \ '_ \ 
//| || (_) | | | | || (_| |  __/ | | |
//|_| \___/|_| |_|\__\__, |\___|_| |_|
//                    __/ |           
//                   |___/  v1.0
                  
/*
* MIT License
*
* Copyright (c) 2020 Seth Ballantyne
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.IO;

namespace fontgen
{
    class Program
    {
        static void Main(string[] args)
        {
            using(StreamWriter sw = File.CreateText("output.txt"))
            {
                using (Bitmap font = new Bitmap("default_font.png"))
                {
                    byte charCounter = 32;
                    int glyphWidth = 10;
                    int glyphHeight = 14;
                    int numChars = 95;
                    int xpos = 3;

                    sw.WriteLine("const char defaultFont[DEFAULT_FONT_NUM_GLYPHS][DEFAULT_FONT_HEIGHT][DEFAULT_FONT_WIDTH] = {");
                    
                    for (int i = 0; i < numChars; i++) 
                    {
                        sw.WriteLine("\t{");
                        sw.WriteLine("\t\t/* {0} - {1} */", (char)charCounter, charCounter);

                        for(int j = 2; j < glyphHeight + 2; j++) // Y axis
                        {
                            sw.Write("\t\t{");
                            for (int k = xpos; k < xpos + glyphWidth; k++) // X axis
                            {
                                Color pixelColour = font.GetPixel(k, j);
                                
                                // individual RGB components are compared because Color.Black
                                // and Color.White return values that contain an alpha value of 255.
                                // it's possible when reading a pixel that an alpha value of 0 will be returned
                                // for an otherwise valid black or white pixel, and the comparison will
                                // will evaluate to false because of it. So, we're ignoring the alpha value.
                                if (pixelColour.R == 0 && pixelColour.G == 0 && pixelColour.B == 0) // character pixel, black
                                {
                                    sw.Write(".#',");
                                }
                                else if (pixelColour.R == 255 && pixelColour.G == 255 && 
                                    pixelColour.B == 255) // transparency pixel, white
                                {
                                    sw.Write("'.',"); 
                                }
                            } // end X axis loop

                            sw.WriteLine("},");
                        } // end Y axis loop

                        xpos = (xpos + glyphWidth + 3);
                        charCounter++;
                        sw.WriteLine("\t},");
                    } // end character loop

                    sw.WriteLine("};");
                }
            }
        }
    }
}

//      _        #
//    _|_|_      #
//    (o o)      #
//ooO--(_)--Ooo--#
