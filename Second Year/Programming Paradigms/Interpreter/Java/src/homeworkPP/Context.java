package homeworkPP;

import java.util.HashMap;

public class Context {
	// Map for variables and their values
	private HashMap<String, Integer> variables;

	Context() {
		variables = new HashMap<String, Integer>();
	}

	// Add a variable to the context
	public void add(String v, Integer i) {
		variables.put(v, i);
	}

	// Treat undefined variable problem using exceptions
	public Integer valueOf(String v) {
		if (variables.containsKey(v))
			return variables.get(v);
		throw new RuntimeException();
	}
	
	// Remove a variable from the context
	public void remove(String key){
		variables.remove(key);
	}
}