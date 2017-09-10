package decorator;

import java.util.ArrayList;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Assignment program
public class AttProg extends Prog {
	private String s;

	public String getS() {
		return s;
	}

	public AttProg() {
		s = "";
	}

	public AttProg(String s) {
		this.s = s;
	}

	@Override
	public void eval(Context c) {
		// Extract the variable and its value
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Add the new value of the variable to the context
		c.add(list[0], Main.evalExpression(list[1], c));
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c, ArrayList<String> l) {
		v.visit(this, c, l);
		// Extract the left and right hand sides
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to the right hand side
		Elem.type1(list[1]).accept(v, c);
		// If the variable is not in the context add it to the local variables
		// list
		try {
			c.valueOf(list[0]);
		} catch (RuntimeException e) {
			l.add(list[0]);
		}
		// Add the variable to the context
		c.add(list[0], 1);
		// Apply accept to the variable
		Elem.type1(list[0]).accept(v, c);
	}
}