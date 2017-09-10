package decorator;

import java.util.ArrayList;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Return program
public class ReturnProg extends Prog {
	private String s;

	public String getS() {
		return s;
	}

	public ReturnProg() {
		s = "";
	}

	public ReturnProg(String s) {
		this.s = s;
	}

	@Override
	public void eval(Context c) {
		// Extract the value to be returned
		String expr = s.substring(8, s.length() - 1);
		// Add 'return' to the context as a variable
		c.add("return", Main.evalExpression(expr, c));
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c, ArrayList<String> l) {
		v.visit(this, c, l);
		// Extract the value to be returned
		String expr = s.substring(8, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to the value
		Elem.type1(list[0]).accept(v, c);
		// Add return to the context as a variable
		c.add("return", 1);
		// Check that the number of the total open parenthesis is equal to the
		// closed ones, so that return is the last program to be executed
		if (Main.paranthesis != 0)
			Main.correctProgram = false;
	}
}
