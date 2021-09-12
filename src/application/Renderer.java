package application;

import java.util.ArrayList;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

public class Renderer {
    Window window;
    StyledDocument sdoc;
    public Renderer(Window window, JTextPane textpane){
        this.window = window;
        this.sdoc = textpane.getStyledDocument();
    }
    public void renderHTML(ArrayList<Element> document, int id, Style oldStyle){
        Element elm = document.get(id);
        Style newStyle = oldStyle;
        try{
            switch(elm.name){
                // metadata
                case "title":
                    String title = document.get(elm.childElements.get(0)).attributes.get(0);
                    window.setTitle(title);
                    return;
                case "style":
                case "script":
                    return;
                // flow
                //case "a":
                case "br":
                    sdoc.insertString(sdoc.getLength(), "\n", newStyle);
                    break;
                case "h1":
                    StyleConstants.setFontSize(newStyle, 36);
                    break;
                case "h2":
                    StyleConstants.setFontSize(newStyle, 36);
                    break;
                case "h3":
                    StyleConstants.setFontSize(newStyle, 24);
                    break;
                case "h4":
                    StyleConstants.setFontSize(newStyle, 16);
                    break;
                case "h5":
                    StyleConstants.setFontSize(newStyle, 16);
                    break;
                case "h6":
                    StyleConstants.setFontSize(newStyle, 16);
                    break;
                // sectioning
                // heading
                // phrasing
                // embedded
                // interactive
                // text
                case "text":
                    sdoc.insertString(sdoc.getLength(), elm.attributes.get(0), newStyle);
                    break;
            }
        }catch(Exception e){}

        for(int childElementId : elm.childElements){
            renderHTML(document, childElementId, newStyle);
        }
    }
}