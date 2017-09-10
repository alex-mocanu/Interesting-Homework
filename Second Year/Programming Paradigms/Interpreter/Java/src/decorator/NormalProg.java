package decorator;

import java.util.ArrayList;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Normal sequential program
public class NormalProg extends Prog {
	private String s;

	public String getS() {
		return s;
	}

	public NormalProg() {
		s = "";
	}

	public NormalProg(String s) {
		this.s = s;
	}

	@Override
	public void eval(Context c) {
		// Extract the programs
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);

		// Evaluate each program
		Prog p1 = type(list[0]);
		Prog p2 = type(list[1]);
		p1.eval(c);
		p2.eval(c);
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c, ArrayList<String> l) {
		v.visit(this, c, l);
		// Extract the programs
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to each program
		Prog.type(list[0]).accept(v, c, l);
		Prog.type(list[1]).accept(v, c, l);
	}
}
