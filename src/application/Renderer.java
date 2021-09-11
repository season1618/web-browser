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
        Element elm = document.get(id);System.out.print(elm.name);
        switch(elm.name){
            // metadata
            case "title":
                String title = document.get(elm.childElements.get(0)).attributes.get(0);
                window.setTitle(title);
                return;
            // flow
            // sectioning
            // heading
            // phrasing
            // embedded
            // interactive
        }
        for(int childElementId : elm.childElements){
            renderHTML(document, childElementId);
        }
    }
}