package window;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

import java.util.ArrayList;

import window.reader.Element;
import window.reader.Parser;
//import parser.Parser;

public class Window extends JFrame implements WindowListener, ActionListener {
    TextField tf;
	JScrollPane scrollpane;
    JTextPane textpane;
	public Window(){
		super();

		Rectangle region = new Rectangle(300, 100, 900, 600);
		
		this.setTitle("window");
		this.setBounds(region);
		this.setLayout(null);
		this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		//JTabbedPane tabclass = new JTabbedPane();
		/*JPanel tab = new JPanel();
		tab.setBounds(0,0,state.width,24);
		tab.setLayout(null);
		add(tab);*/

		tf = new TextField();
		tf.setBounds(100, 0, Math.min(300, region.width - 215), 24);
		tf.setFont(new Font("MS Gothic",Font.PLAIN,16));
		tf.addActionListener(this);
		this.add(tf);

		scrollpane = new JScrollPane();
		scrollpane.setBounds(0, 24, this.getWidth() - 14, this.getHeight() - 60);
		this.add(scrollpane);

		textpane = new JTextPane();
		textpane.setEditable(false);
		//textpane.addDocumentListener(this);

		JViewport viewport = new JViewport();
		viewport.setView(textpane);
		scrollpane.setViewportView(viewport);

		//JEditorPane editorpane = new JEditorPane();
		//editorpane.setEditable(false);
		//editorpane.setText("hello");

		this.addWindowListener(this);
		this.addWindowFocusListener(
            new WindowAdapter(){
                public void windowGainedFocus(WindowEvent e) {
                    requestFocusInWindow();
                }
            }
        );

		this.setVisible(true);
	}

	public void windowOpened(WindowEvent e){}
	public void windowClosing(WindowEvent e){ dispose(); }
	public void windowClosed(WindowEvent e){}
	public void windowIconified(WindowEvent e){}
	public void windowDeiconified(WindowEvent e){}
	public void windowActivated(WindowEvent e){}
	public void windowDeactivated(WindowEvent e){}

	public void actionPerformed(ActionEvent ev){
		if(ev.getSource() == tf){
			String urlString = tf.getText();
			ArrayList<Element> document = new ArrayList<Element>(1);
			Parser parser = new Parser(urlString);
			parser.parseHTML(document, 0);
			for(Element elm:document){
				System.out.print(elm.name);
				for(String s:elm.attributes) System.out.print(" " + s);
				System.out.println();
			}
			//textpane.setText("");
			//StyledDocument doc = textpane.getStyledDocument();
			//Style style = doc.getStyle(StyleContext.DEFAULT_STYLE);
		}
	}
}