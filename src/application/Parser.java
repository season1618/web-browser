package application;

import java.util.ArrayList;
import java.util.Arrays;

import java.net.URL;
import java.net.HttpURLConnection;
import java.net.UnknownHostException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class Parser {
    InputStreamReader isr;
    ArrayList<String> emptyElementName = new ArrayList<String>(
        Arrays.asList("!DOCTYPE", "area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr")
    );
    public Parser(String urlString){
        try{
            URL url = new URL(urlString);
            HttpURLConnection con = (HttpURLConnection)url.openConnection();
            con.setRequestMethod("GET");
            con.setRequestProperty("User-Agent", "Mozilla/5.0");
            if(con.getResponseCode() != 200){
                System.out.println(con.getResponseCode());
            }

            isr = new InputStreamReader(con.getInputStream(), "UTF-8");
        }catch(Exception e){}
    }
    public void parseHTML(ArrayList<Element> document, int id){
        try{
            int i; char c; int count = 10000;
            while(count-- > 0){
                i = isr.read();
                if(i == -1) break;
                c = (char)i;
                if(c == '<'){ // element
                    Element elm = parseTag();
                    if(elm.name.charAt(0) != '/'){ // opening tag
                        int child_id = document.size();
                        document.add(elm);
                        document.get(id).childElements.add(child_id);
                        if(!emptyElementName.contains(elm.name)) parseHTML(document, child_id);
                    }else{ // closing tag
                        return;
                    }
                }else{ // text
                    int n = document.size();
                    if(document.get(n-1).name.equals("text")){
                        String s = document.get(n-1).attributes.get(0);
                        if(c == '\n');
                        else if(c == ' ' && s.charAt(s.length()-1) == ' ');
                        else document.get(n-1).attributes.set(0, document.get(n-1).attributes.get(0).concat(String.valueOf(c)));
                    }else if(c != '\n' && c != ' '){
                        int child_id = document.size();
                        document.add(new Element("text", String.valueOf(c)));
                        document.get(id).childElements.add(child_id);
                    }
                }
            }
        }catch(Exception e){}
    }
    public Element parseTag(){
        Element elm = new Element();
        try{
            char c;
            // name
            while(true){
                c = (char)isr.read();
                if(c == ' ') break;
                if(c == '>') return elm;
                elm.name = elm.name.concat(String.valueOf(c));
            }

            // attribute, value
            while(true){
                while(true){
                    c = (char)isr.read();
                    if(c == '>') return elm;
                    if(c != ' ' && c != '=') break;
                }

                if(c == '\"'){
                    elm.attributes.add("");
                    int n = elm.attributes.size();
                    while(true){
                        c = (char)isr.read();
                        if(c == '\"') break;
                        elm.attributes.set(n-1, elm.attributes.get(n-1).concat(String.valueOf(c)));
                    }
                }else{
                    elm.attributes.add(String.valueOf(c));
                    int n = elm.attributes.size();
                    while(true){
                        c = (char)isr.read();
                        if(c == ' ' || c == '=') break;
                        if(c == '>') return elm;
                        elm.attributes.set(n-1, elm.attributes.get(n-1).concat(String.valueOf(c)));
                    }
                }
            }
        }catch(Exception e){}
        return elm;
    }
}