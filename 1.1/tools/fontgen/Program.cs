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
        // the name and path of the image containing the bitmap font
        static string fontFile = "default_font.png";

        // the path the output should be written to
        static string output = "output.txt";

        // the number of characters in the font
        static int numChars = 95;

        // the pixel width of each individual character
        static byte glyphWidth = 10;

        // the pixel height of each individual character
        static byte glyphHeight = 14;

        // the decimal value of the first character in the font.
        // used for writing a comment at the top of each character
        // in the output file.
        static byte charCounter = 32;

        static void ExceptionPrompt(string message)
        {
            Console.WriteLine("Error: " + message);
            Console.ReadLine();
        }

        static void ParseCommandLineArgs(string[] args)
        {
            foreach (string s in args)
            {
                string[] splitStr = s.Split('=');
                splitStr[0] = splitStr[0].ToLower();

                if (splitStr.Length > 1)
                {
                    switch (splitStr[0])
                    {
                        case "font":
                            if (!String.IsNullOrEmpty(splitStr[1]))
                            {
                                fontFile = splitStr[1];
                            }
                            break;
                        case "output":
                            if (!String.IsNullOrEmpty(splitStr[1]))
                            {
                                output = splitStr[1];
                            }
                            break;
                        case "numchars":
                            try
                            {
                                numChars = Convert.ToInt32(splitStr[1]);
                            }
                            catch
                            {
                                Console.WriteLine("the numchars argument contains an invalid value; using defaults.");
                            }
                            break;
                        case "width":
                            try
                            {
                                glyphWidth = Convert.ToByte(splitStr[1]);
                            }
                            catch
                            {
                                Console.WriteLine("the width argument contains an invalid value; using defaults.");
                            }
                            break;
                        case "height":
                            try
                            {
                                glyphHeight = Convert.ToByte(splitStr[1]);
                            }
                            catch
                            {
                                Console.WriteLine("the width argument contains an invalid value; using defaults.");
                            }
                            break;
                        case "initial":
                            try
                            {
                                charCounter = Convert.ToByte(splitStr[1]);
                            }
                            catch
                            {
                                Console.WriteLine("the initial argument contains an invalid value; using defaults.");
                            }
                            break;
                        default:
                            Console.WriteLine("Unknown argument '{0}'", splitStr[0]);
                            break;
                    }
                }
            }
        }

        static void Main(string[] args)
        {
            try
            {
                Console.WriteLine("  __            _   ");
                Console.WriteLine(" / _|          | |  ");
                Console.WriteLine("| |_ ___  _ __ | |_ __ _  ___ _ __  ");
                Console.WriteLine("|  _/ _ \\| '_ \\| __/ _` |/ _ \\ '_ \\ ");
                Console.WriteLine("| || (_) | | | | || (_| |  __/ | | |");
                Console.WriteLine("|_| \\___/|_| |_|\\__\\__, |\\___|_| |_|");
                Console.WriteLine("                    __/ |           ");
                Console.WriteLine("                   |___/  v1.0      ");
                Console.WriteLine();

                ParseCommandLineArgs(args);

                using (StreamWriter sw = File.CreateText(output))
                {
                    using (Bitmap font = new Bitmap(fontFile))
                    {
                        // basic error checking.
                        // if we don't do this, Bitmap.GetPixel() will generate an ArgumentException
                        // when trying to access a pixel outside the images dimenions and I want to avoid
                        // that, because the handler for ArgumentException is catered towards Bitmap() or
                        // File.CreateText() failing. See the comment above the catch statements as to why.
                        // Docs claim Bitmap.GetPixel() generates an ArgumentOutOfRange Exception, but it's not.
                        // Unlike when we parse the command line args, the variables in question aren't set to 
                        // their defaults, because it's possible it'll result in garbage output anyway. 
                        if (glyphWidth * numChars > font.Width)
                        {
                            throw new Exception("the specified value(s) for the character width and/or the number of characters used by the font are too big.");
                        }
                        else if (glyphHeight > font.Height)
                        {
                            throw new Exception("the specified height value is greater than the height of the font image.");
                        }

                        Console.WriteLine("Using font {0}", fontFile);
                        Console.WriteLine("Writing output to {0}", output);
                        Console.WriteLine("number of characters: {0}", numChars);
                        Console.WriteLine("individual character width: {0}", glyphWidth);
                        Console.WriteLine("individual character height: {0}", glyphHeight);
                        Console.WriteLine();

                        int xpos = 3;

                        sw.WriteLine("#define DEFAULT_FONT_HEIGHT\t{0}", glyphHeight);
                        sw.WriteLine("#define DEFAULT_FONT_WIDTH\t{0}", glyphWidth);
                        sw.WriteLine("#define DEFAULT_FONT_NUM_GLYPHS {0}", numChars);
                        sw.WriteLine("#define DEFAULT_FONT_FIRST_CHAR {0}", charCounter);
                        sw.WriteLine("#define DEFAULT_FONT_LAST_CHAR {0}", charCounter + numChars - 1);
                        sw.WriteLine();
                        sw.WriteLine("const char defaultFont[DEFAULT_FONT_NUM_GLYPHS][DEFAULT_FONT_HEIGHT][DEFAULT_FONT_WIDTH] = {");

                        for (int i = 0; i < numChars; i++)
                        {
                            sw.WriteLine("\t{");
                            sw.WriteLine("\t\t/* '{0}' - decimal {1} */", (char)charCounter, charCounter);

                            for (int j = 2; j < glyphHeight + 2; j++) // Y axis
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
                                        sw.Write("'#',");
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

                Console.WriteLine("All done!");
            }

            // the documentation for the Bitmap constructor states that it throws
            // a FileNotFoundException if the image can't be found, but it's actually
            // throwing an ArgumentException. I'm handling both to be safe.
            // The handler for ArgumentException has a slightly different message
            // from the one in the FileNotFoundException handler, because File.CreateText can throw it too.
            catch (FileNotFoundException)
            {
                ExceptionPrompt("The specified font image couldn't be found.");
            }
            catch (ArgumentException)
            {
                // ArgumentException generates a message along the lines of "the parameter is invalid"
                // and it's not helpful in the least.
                ExceptionPrompt("the font image is missing or the output file is an invalid filename.");
            }
            catch (Exception e)
            {
                ExceptionPrompt(e.Message);
            }
        }
    }
}

//      _        #
//    _|_|_  boO!#
//    (o o)      #
//ooO--(_)--Ooo--#

