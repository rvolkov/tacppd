import java.awt.*;
import java.applet.*;

public class test extends Applet {
	Image[] I;
	AudioClip[] A;
  public void init() {
		// 0. set applet area size
		resize(900,900);

		// set applet background color
		setBackground(Color.white);

	}

	public void paint(Graphics g) {

		// 1. example of line painting
		for(int i=0; i<100; i+=10) {
			g.drawLine(i, i, i+200, i);
		}
		
		// 2. example of rectangle
		for(int i=0; i<140; i+=20) {
			g.drawRect(i,5,15,45);
		}
		for(int i=0; i<140; i+=20) {
			g.fillRect(i,55,15,45);
		}
		for(int i=0; i<140; i+=20) {
			g.drawRoundRect(i,100,20,45,10,15);
		}
		for(int i=0; i<100; i+=20) {
			g.draw3DRect(i,150,15,35,true);
		}
		
		// 3. gif image output
		I = new Image[1];
		I[0] = getImage(getDocumentBase(),"cisco.gif");
		g.drawImage(I[0], 20, 20, this);

		// 4. audio
		play(getDocumentBase(),"beep.au");

		// 5. text output
		int w = 200;
		int h = 150;

		Font f1 = new Font("Courier", Font.PLAIN, 18);
		g.setFont(f1);
		FontMetrics fm1 = g.getFontMetrics();
		g.drawString("Courier", 11, 10+fm1.getHeight());

		Font f2 = new Font("TimesRoman", Font.ITALIC, 12);
		g.setFont(f2);
		FontMetrics fm2 = g.getFontMetrics();
		g.drawString("TimesRoman", w-fm2.stringWidth("TimesRoman"),
			10+fm2.getHeight());

		Font f3 = new Font("Symbol", Font.BOLD, 18);
		g.setFont(f3);
		FontMetrics fm3 = g.getFontMetrics();
		g.drawString("Symbol", 10, h);

		Font f4 = new Font("Dialog", Font.PLAIN, 18);
		g.setFont(f4);
		FontMetrics fm4 = g.getFontMetrics();
		g.drawString("Courier", w-fm4.stringWidth("Dialog"), h);
	}
}
