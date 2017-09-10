package decorator;

import java.util.ArrayList;
import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// While program
public class WhileProg extends Prog {
	private String s;

	public String getS() {
		return s;
	}

	public WhileProg() {
		s = "";
	}

	public WhileProg(String s) {
		this.s = s;
	}

	@Override
	public void eval(Context c) {
		// Extract the halting condition and the block
		String expr = s.substring(7, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Establish the type of condition
		Elem<Boolean> val;
		if (list[0].charAt(1) == '=')
			val = new EqualExpr(list[0]);
		else
			val = new CompExpr(list[0]);
		// Evaluate block while condition is met
		while (val.eval(c)) {
			Prog p = type(list[1]);
			p.eval(c);
		}
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c, ArrayList<String> l) {
		// List for local variables inside while program
		ArrayList<String> l2 = new ArrayList<String>();
		// Begin visit of while program
		v.startVisit(this, c, l2);
		// Extract the condition and the block
		String expr = s.substring(7, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to the condition and the block
		Elem.type1(list[0]).accept(v, c);
		Prog.type(list[1]).accept(v, c, l2);
		// End the visit of the while program
		v.endVisit(this, c, l2);
	}
}
