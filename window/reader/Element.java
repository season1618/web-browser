package window.reader;

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
}