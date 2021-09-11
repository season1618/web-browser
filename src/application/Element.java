package application;

import java.util.ArrayList;
import java.util.Arrays;

public class Element {
    public String name;
    public ArrayList<String> attributes;
    public ArrayList<Integer> childElements;
    public Element() {
        name = "";
        attributes = new ArrayList<String>();
        childElements = new ArrayList<Integer>();
    }
    public Element(String name, String attribute) {
        this.name = new String(name);
        this.attributes = new ArrayList<String>(Arrays.asList(attribute));
        this.childElements = new ArrayList<Integer>();
    }
}