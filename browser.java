import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import java.util.ArrayList;
// HTTP Comunication
import java.net.URL;
import java.net.HttpURLConnection;
import java.net.UnknownHostException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

class frame extends JFrame implements ComponentListener,WindowListener,ActionListener{
	public static ArrayList<frame> fr = new ArrayList<>();
	int id;
	int left; int right;
	TextField tf; Button plus,edit;
	JScrollPane scrollpane;JTextPane textpane;
	public frame(int i , int l , int r , Rectangle state){
		super();
		
		id = i;
		left = l; right = r;
		setTitle("window" + i);
		setBounds(state);
		setLayout(null);
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		//JTabbedPane tabclass = new JTabbedPane();
		/*JPanel tab = new JPanel();
		tab.setBounds(0,0,state.width,24);
		tab.setLayout(null);
		add(tab);*/

		tf = new TextField();
		tf.setBounds(100,0,Math.min(300,state.width - 215),24);
		tf.setFont(new Font("MS Gothic",Font.PLAIN,16));
		tf.addActionListener(this);
		add(tf);

		plus = new Button("+");
		plus.setBounds(tf.getX() + tf.getWidth(),0,40,24);
		plus.addActionListener(this);
		add(plus);

		edit = new Button("Editor");
		edit.setBounds(state.width - 75,0,60,24);
		edit.addActionListener(this);
		add(edit);

		scrollpane = new JScrollPane();
		scrollpane.setBounds(0,24,getWidth() - 14,getHeight() - 60);
		add(scrollpane);

		textpane = new JTextPane();
		textpane.setEditable(false);
		//textpane.addDocumentListener(this);

		JViewport viewport = new JViewport();
		viewport.setView(textpane);
		scrollpane.setViewportView(viewport);

		//JEditorPane editorpane = new JEditorPane();
		//editorpane.setEditable(false);
		//editorpane.setText("hello");

		addComponentListener(this);
		addWindowListener(this);
		addWindowFocusListener(new WindowAdapter(){
    		public void windowGainedFocus(WindowEvent e) {
        		requestFocusInWindow();
    		}
		});

		setVisible(true);
	}
	public static void main(String args[]){
		frame.fr.add(new frame(0,-1,-1,new Rectangle(300,100,900,600)));
	}
	public void componentMoved(ComponentEvent e){
		Point p = getLocation();
		for(int i = left; i > -1; i = frame.fr.get(i).left){
			p.x -= frame.fr.get(i).getWidth();
			frame.fr.get(i).setLocation(p);
		}
		p = getLocation();
		for(int i = id; i > -1; i = frame.fr.get(i).right){
			frame.fr.get(i).setLocation(p);
			p.x += frame.fr.get(i).getWidth();
		}
	}public void componentResized(ComponentEvent e){
		tf.setSize(Math.min(300,getWidth() - 215) , 24);
		plus.setLocation(tf.getX() + tf.getWidth() , 0);
		edit.setLocation(getWidth() - 75 , 0);
		scrollpane.setSize(getWidth() - 14 , getHeight() - 60);
		if(left > -1){
			frame l = frame.fr.get(left);
			l.setSize(this.getX() - l.getX() , this.getHeight());
		}
		if(right > -1){
			frame r = frame.fr.get(right);
			r.setBounds(
				this.getX() + this.getWidth() , this.getY() ,
				r.getX() + r.getWidth() - this.getX() - this.getWidth() , this.getHeight()
			);
		}
	}
	public void componentHidden(ComponentEvent e){}
	public void componentShown(ComponentEvent e){}

	public void windowOpened(WindowEvent e){}
	public void windowClosing(WindowEvent e){
		if(left > -1){
			frame.fr.get(left).right = right;
			frame.fr.get(left).setSize(
				frame.fr.get(left).getWidth() + this.getWidth() ,
				this.getHeight()
			);
		}if(right > -1){
			frame.fr.get(right).left = left;
		}
		dispose();
	}public void windowClosed(WindowEvent e){}
	public void windowIconified(WindowEvent e){
		int s = id; int cnt = 0;
		for(int i = id; i > -1; i = frame.fr.get(i).left){ cnt++; s = i; }
		for(int i = id; i > -1; i = frame.fr.get(i).right) cnt++;
		int n = cnt - 1;
		Toolkit tlk = Toolkit.getDefaultToolkit();
		int w = (int)tlk.getScreenSize().getWidth(); w /= n;
		int h = (int)tlk.getScreenSize().getHeight();
		int x = 0;
		for(int i = s; i > -1; i = frame.fr.get(i).right){
			frame.fr.get(i).setBounds(x , 0 , w , h);
			x += w;
		}this.setState(JFrame.NORMAL);
	}
	public void windowDeiconified(WindowEvent e){}
	public void windowActivated(WindowEvent e){}
	public void windowDeactivated(WindowEvent e){}

	public void actionPerformed(ActionEvent ev){
		if(ev.getSource() == tf){
			String str = tf.getText();
			if(str.equals("end")) System.exit(0);
			else{
				try{
					Rendering(str);
				}catch(Exception e){}
			}
		}else if(ev.getSource() == plus){
			int n = frame.fr.size();
			int temp = right; right = -1;
			int a = (getWidth() - 1)/2; int b = (getWidth() + 1)/2;
			setSize(a , getHeight());
			Rectangle state = getBounds();
			state.x += a;
			state.width = b;
			frame.fr.add(new frame(n,id,temp,state));

			if(temp > -1) frame.fr.get(temp).left = n;
			right = n;
		}else if(ev.getSource() == edit){
			if(textpane.isEditable()){
				textpane.setEditable(false);
				edit.setLabel("Editor");
			}else{
				textpane.setEditable(true);
				edit.setLabel("Browser");
			}
		}
	}
	/*public void changedUpdate(DocumentEvent e){
		String str = "";
		StyledDocument doc = textpane.getStyledDocument();
		Style style = doc.getStyle(StyleContext.DEFAULT_STYLE);
		for(int j = 0; j < doc.getLength(); j++){
			char c = doc.charAt(j);
			if(String.valueOf(c).matches(" |\n|\\(|\\)|\\{|\\}|\\[|\\]")){
				if(str.equals("int")){
					StyleConstants.setForeground(style,Color.red);
					doc.setAttribute(j,3,style,true);
				}
			}
		}
	}*/
	public void Rendering(String str) throws Exception{// HTML Rendering
		textpane.setText("");
		StyledDocument doc = textpane.getStyledDocument();
		Style style = doc.getStyle(StyleContext.DEFAULT_STYLE);

		URL url = new URL(str);
		HttpURLConnection con = (HttpURLConnection)url.openConnection();
		con.setRequestMethod("GET");
		con.setRequestProperty("User-Agent","Mozilla/5.0");
		if(con.getResponseCode() != 200){
			textpane.setText(String.valueOf(con.getResponseCode()));
			return;
		}
		InputStreamReader isr = new InputStreamReader(con.getInputStream(),"UTF-8");

		int i; char c;
		boolean tag = false;
		String tagname = "a"; String body = "";
		String face = "MS Gothic"; int size = 16; Color color = Color.black;
		int depth = 0;
		while(true){
			i = isr.read();
			if(i == -1) break;
			c = (char)i;
			if(c == '&'){// escape processing
				String escape = "";
				while(true){
					i = isr.read();
					c = (char)i;
					if(c == ';') break;
					escape = escape.concat(String.valueOf(c));
				}
				switch(escape){
					case "amp": escape = "&"; break;
					case "lt": escape = "<"; break;
					case "gt": escape = ">"; break;
					case "quot": escape = "\""; break;
					case "#39": escape = "\'"; break;
					case "nbsp": escape = " "; break;
					case "copy": escape = "©"; break;
					default: escape = "";
				}
				body = body.concat(escape);
				continue;
			}
			if(tag){
				if(c == '>'){
					if(tagname.matches("h[1-6].*")){
						int n = tagname.charAt(1) - '0';
						switch(n){
							case 1: size = 28; break;
							case 2: size = 24; break;
							case 3: size = 20; break;
							case 4: size = 16; break;
							case 5: size = 12; break;
							case 6: size = 8; break;
						}
					}else if(tagname.matches("/h[1-6]")){
						face = "MS Gothic";
						size = 16;
						color = Color.black;
						/*try{
							doc.insertString(doc.getLength(),"\n",style);
						}catch(BadLocationException e){}*/
					}
					else if(tagname.matches("font.*")){
						boolean flag = true;
						String variable = "",value = "";
						for(int j = 4; j < tagname.length(); j++){
							char ch = tagname.charAt(j);
							if(ch == ' ') continue;
							else if(ch == '\"'){
								if(flag) flag = false;
								else{
									if(variable.equals("face=")) face = value;
									else if(variable.equals("size=")) size = Integer.valueOf(value);
									else if(variable.equals("color=")){
										switch(value.toLowerCase()){
											case "black": color = Color.black;
											case "red": color = Color.red;
											case "green": color = Color.green;
											case "blue": color = Color.blue;
											case "white": color = Color.white;
										}
									}
									flag = true;
									variable = ""; value = "";
								}
							}else{
								if(flag) variable = variable.concat(String.valueOf(ch));
								else value = value.concat(String.valueOf(ch));
							}
						}
					}else if(tagname.equals("/font")){
						face = "MS Gothic";
						size = 16;
						color = Color.black;
					}
					else if(tagname.equals("b")) StyleConstants.setBold(style,true);
					else if(tagname.equals("/b")) StyleConstants.setBold(style,false);
					else if(tagname.equals("i")) StyleConstants.setItalic(style,true);
					else if(tagname.equals("/i")) StyleConstants.setItalic(style,false);
					else if(tagname.equals("/p")){
						try{
							doc.insertString(doc.getLength(),"\n",style);
						}catch(BadLocationException e){}
					}
					else if(tagname.matches("link.*"));
					else if(tagname.matches("li.*")){
						depth++;
						try{
							for(int j = 0; j < depth; j++) doc.insertString(doc.getLength(),"    ",style);
						}catch(BadLocationException e){}
					}else if(tagname.equals("/li")) depth--;
					tag = false;
					body = "";
				}else tagname = tagname.concat(String.valueOf(c));
			}else{
				if(c == '<'){
					if(tagname.equals("title")) setTitle(body);
					else if(tagname.contains("script"));
					else if(tagname.matches("a .*")){
						boolean flag = true;
						String variable = "",value = "";
						for(int j = 2; j < tagname.length(); j++){
							char ch = tagname.charAt(j);
							if(ch == ' ') continue;
							else if(ch == '\"'){
								if(flag) flag = false;
								else{
									if(variable.equals("href=")){
										String s = url.getProtocol() + "://" + url.getHost();
										if(!value.contains(s)) value = s.concat(value);
										Hyperlink link = new Hyperlink(value,body);
										link.setFont(new Font(face,Font.PLAIN,size));
										textpane.insertComponent(link);
									}
									flag = true;
									variable = ""; value = "";
								}
							}else{
								if(flag) variable = variable.concat(String.valueOf(ch));
								else value = value.concat(String.valueOf(ch));
							}
						}
					}
					else if(body.isEmpty());
					else{
						StyleConstants.setFontFamily(style,face);
						StyleConstants.setFontSize(style,size);
						StyleConstants.setForeground(style,color);
						try{
							doc.insertString(doc.getLength(),body,style);
						}catch(BadLocationException e){}
					}
					tag = true;
					tagname = "";
				}else body = body.concat(String.valueOf(c));
			}
		}
	}
	class Hyperlink extends JLabel{
		String href;
		public Hyperlink(String url,String label){
			super();
			setText(label);
			href = url;
			setForeground(Color.blue);
			addMouseListener(new MouseAdapter(){
				public void mousePressed(MouseEvent e){
					tf.setText(href);
					try{
						Rendering(href);
					}catch(Exception ex){}
				}
			});
		}
	}
}

/*class MyStyledDocument implements StyledDocument{
	String face;
	int size;
	Color color;
	public MyStyledDocument(){
		face = "MS Gothic";
		size = 16;
		color = Color.black;
	}
	public void append(String str,Style style){
		StyleConstants.setFontFamily(style,face);
		StyleConstants.setFontSize(style,size);
		StyleConstants.setForeground(style,color);
		try{
			insertString(getLength(),str,style);
		}catch(BadLocationException e){}
	}
	Style addStyle(String nm,Style parent)
	Color getBackground()
	Font getFont(AttributeSet as){}
}*/