package application;

import java.util.ArrayList;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

public class Renderer {
    Window window;
    JTextPane textpane;
    StyledDocument sdoc;
	Style style;
    public Renderer(Window window, JTextPane textpane){
        this.window = window;
        this.textpane = textpane;
        this.sdoc = textpane.getStyledDocument();
        this.style = sdoc.getStyle(StyleContext.DEFAULT_STYLE);
    }
    public void renderHTML(ArrayList<Element> document, int id){
        Element elm = document.get(id);
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
                    sdoc.insertString(sdoc.getLength(), "\n", style);
                    break;
                // sectioning
                // heading
                // phrasing
                // embedded
                // interactive
                // text
                case "text":
                    sdoc.insertString(sdoc.getLength(), elm.attributes.get(0), style);
                    break;
            }
        }catch(Exception e){}

        for(int childElementId : elm.childElements){
            renderHTML(document, childElementId);
        }
    }
}