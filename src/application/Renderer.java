package application;

import java.util.ArrayList;
import java.util.Arrays;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

public class Renderer {
    Window window;
    StyledDocument sdoc;
    ArrayList<Element> document;
    ArrayList<String> blockLevelElementList = new ArrayList<String>(
        Arrays.asList("div", "h1", "h2", "h3", "h4", "h5", "h6", "li", "p", "ul")
    );
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
                    renderList(id, 0, newStyle);
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

        try{
            if(blockLevelElementList.contains(elm.name)){
                sdoc.insertString(sdoc.getLength(), "\n", newStyle);
            }
        }catch(BadLocationException e){}
    }

    private void renderList(int id, int depth, Style style){
        Element elm = document.get(id);
        try{
            sdoc.insertString(sdoc.getLength(), "\n", style);
        }catch(BadLocationException e){}
        for(int childElementId : elm.childElements){
            Element e = document.get(childElementId);
            System.out.print(e.name);
            for(String a : e.attributes) System.out.print(" a" + a + "a");
            System.out.print("\n");
            renderItem(childElementId, depth + 1, style);
        }
    }
    private void renderItem(int id, int depth, Style style){
        Element elm = document.get(id);
        String s = "";
        for(int i = 0; i < depth; i++) s += "    ";
        s += "\u2022"; // bullet
        try{
            sdoc.insertString(sdoc.getLength(), s, style);
        }catch(BadLocationException e){}

        boolean isLineFeed = true;
        for(int childElementId : elm.childElements){
            Element childElm = document.get(childElementId);
            switch(childElm.name){
                case "ul":
                    isLineFeed = false;
                    renderList(childElementId, depth, style);
                    break;
                default:
                    renderHTML(childElementId, style);
                    break;
            }
        }
        if(isLineFeed){
            try{
                sdoc.insertString(sdoc.getLength(), "\n", style);
            }catch(BadLocationException e){}
        }
    }
}