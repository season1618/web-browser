package application;

import java.util.ArrayList;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

public class Renderer {
    Window window;
    StyledDocument sdoc;
    ArrayList<Element> document;
    public Renderer(Window window, JTextPane textpane, ArrayList<Element> document){
        this.window = window;
        this.sdoc = textpane.getStyledDocument();
        this.document = document;
    }
    public void renderHTML(int id, Style oldStyle){
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

                case "p":
                    StyleConstants.setFontSize(newStyle, 16);
                    break;
                case "ul":
                    StyleConstants.setFontSize(newStyle, 16);
                    renderList(sdoc, id, 0, newStyle);
                    return;
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
            renderHTML(childElementId, newStyle);
        }
    }

    private void renderList(StyledDocument sdoc, int id, int depth, Style style){
        Element elm = document.get(id);
        for(int childElementId : elm.childElements){
            renderItem(sdoc, childElementId, depth + 1, style);
        }
    }
    private void renderItem(StyledDocument sdoc, int id, int depth, Style style){
        Element elm = document.get(id);
        for(int childElementId : elm.childElements){
            Element childElm = document.get(childElementId);
            switch(childElm.name){
                case "text":
                    String s = "";
                    for(int i = 0; i < depth; i++) s += "    ";
                    s += "\u2022"; // bullet
                    try{
                        sdoc.insertString(sdoc.getLength(), s + childElm.attributes.get(0), style);
                    }catch(BadLocationException e){}
                    break;
                case "ul":
                    renderList(sdoc, childElementId, depth, style);
                    break;
            }
        }
    }
}