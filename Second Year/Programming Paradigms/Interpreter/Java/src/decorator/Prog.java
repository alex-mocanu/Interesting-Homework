package decorator;

import java.util.ArrayList;
import homeworkPP.Context;
import visitor.Visitor;

// Abstract class for programs
public abstract class Prog {
	public abstract void eval(Context c);

	/**
	 * Accept function for Visitor design pattern
	 * 
	 * @param v
	 *            - a visitor object
	 * @param c
	 *            - the current context
	 * @param l
	 *            - a list of local variables names
	 */
	public abstract void accept(Visitor v, Context c, ArrayList<String> l);

	// Decide the type of program
	public static Prog type(String s) {
		switch (s.charAt(1)) {
		case '=':
			return new AttProg(s);
		case 'i':
			return new IfProg(s);
		case 'w':
			return new WhileProg(s);
		case ';':
			return new NormalProg(s);
		default:
			return new ReturnProg(s);
		}
	}
}