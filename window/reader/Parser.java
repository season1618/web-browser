package window.reader;

import java.util.ArrayList;
import java.util.Arrays;
import window.reader.Element;

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
            int i; char c; int count = 1000;
            while(count-- > 0){
                i = isr.read();
                if(i == -1) break;
                c = (char)i;
                if(c == '<'){ // element
                    Element elm = parseTag();
                    if(elm.name.charAt(0) != '/'){ // opening tag
                        int child_id = document.size();
                        document.add(new Element());
                        document.set(child_id, elm);
                        document.get(id).childElements.add(child_id);
                        if(!emptyElementName.contains(elm.name)) parseHTML(document, child_id);
                    }else{
                        return;
                    }
                }else{ // text
                    int n = document.size();
                    if(document.get(n-1).name.equals("text")){
                        document.get(n-1).attributes.set(0, document.get(n-1).attributes.get(0).concat(String.valueOf(c)));
                    }else if(c != '\n'){
                        int child_id = document.size();
                        document.add(new Element());
                        document.get(child_id).name = "text";
                        document.get(child_id).attributes.add(String.valueOf(c));
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