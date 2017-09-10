package interpretor;

/**
 * Term node
 * 
 * @author alexm
 */
public class Termeni extends Nod {
	Termeni() {
		this(null);
	}

	Termeni(String val) {
		setName('T');
		this.setVal(val);
	}
}