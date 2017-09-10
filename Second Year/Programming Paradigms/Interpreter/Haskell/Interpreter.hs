module Interpreter
  (
    -- * Types
    Prog,

    -- * Functions
    evalRaw,
    evalAdt,
  ) where

-------------------------------------------------------------------------------
--------------------------------- The Expr ADT  -------------------------------
-------------------------------------------------------------------------------
data Expr = Add Expr Expr
          | Sub Expr Expr
          | Mult Expr Expr
          | Equal Expr Expr
          | Smaller Expr Expr
          | Symbol String
          | Value Int deriving (Show, Read)

-- [Optional] TODO Implement a parser for the Expr ADT.
--
parseExpr :: String -> Maybe Prog

-------------------------------------------------------------------------------
---------------------------------- The Prog ADT -------------------------------
-------------------------------------------------------------------------------
data Prog = Eq String Expr
          | Seq Prog Prog
          | If Expr Prog Prog
          | While Expr Prog
          | Return Expr deriving (Show, Read)

-- [Optional] TODO Implement a parser for the Prog ADT.
--


-- [Optional] TODO The *parse* function.  It receives a String - the program in
-- a "raw" format and it could return *Just* a program as an instance of the
-- *Prog* data type if no parsing errors are encountered, or Nothing if parsing
-- failed.
--
-- This is composed with evalAdt to yield the evalRaw function.
parse :: String -> Maybe Prog
parse = undefined

-------------------------------------------------------------------------------
-------------------------------- The Interpreter ------------------------------
-------------------------------------------------------------------------------

-- TODO The *evalAdt* function.  It receives the program as an instance of the
-- *Prog* data type and returns an instance of *Either String Int*; that is,
-- the result of interpreting the program.
--
-- The result of a correct program is always an Int.  This is a simplification
-- we make in order to ease the implementation.  However, we wrap this Int with
-- a *Either String* type constructor to handle errors.  The *Either* type
-- constructor is defined as:
--
-- data Either a b = Left a | Right b
--
-- and it is generally used for error handling.  That means that a value of
-- *Left a* - Left String in our case - wraps an error while a value of *Right
-- b* - Right Int in our case - wraps a correct result (notice that Right is a
-- synonym for "correct" in English).
-- 
-- For further information on Either, see the references in the statement of
-- the assignment.
--

--pair data structure for key-value mapping--
data Pair a b = Cons a b
first :: (Pair a b) -> a
first (Cons a b) = a
second :: (Pair a b) -> b
second (Cons a b) = b

--adding key-value element to map--
addToList :: (Pair String Int) -> [Pair String Int] -> [Pair String Int]
addToList pair [] = [pair]
addToList pair (h:t) = if (first pair) == (first h) then pair:t else h:(addToList pair t)

--evaluate an expression--
evalExpr :: Expr -> [Pair String Int] -> Either String Int
evalExpr (Value a) l = Right a
evalExpr (Symbol s) [] = Left "Uninitialized variable"
evalExpr (Symbol s) (h:t) = if s == (first h) then Right (second h) else evalExpr (Symbol s) t
evalExpr (Add expr1 expr2) l = case evalExpr expr1 l of
                                Left msg -> Left msg
                                Right val1 ->
                                  case evalExpr expr2 l of
                                    Left msg -> Left msg
                                    Right val2 -> Right ((+) val1 val2)
evalExpr (Sub expr1 expr2) l = case evalExpr expr1 l of
                                Left msg -> Left msg
                                Right val1 ->
                                  case evalExpr expr2 l of
                                    Left msg -> Left msg
                                    Right val2 -> Right ((-) val1 val2)
evalExpr (Mult expr1 expr2) l = case evalExpr expr1 l of
                                Left msg -> Left msg
                                Right val1 ->
                                  case evalExpr expr2 l of
                                    Left msg -> Left msg
                                    Right val2 -> Right ((*) val1 val2)
evalExpr (Equal expr1 expr2) l = case evalExpr expr1 l of
                                Left msg -> Left msg
                                Right val1 ->
                                  case evalExpr expr2 l of
                                    Left msg -> Left msg
                                    Right val2 -> Right (if val1 == val2 then 1 else 0)
evalExpr (Smaller expr1 expr2) l = case evalExpr expr1 l of
                                Left msg -> Left msg
                                Right val1 ->
                                  case evalExpr expr2 l of
                                    Left msg -> Left msg
                                    Right val2 -> Right (if val1 < val2 then 1 else 0)

--execute program to return a key-value mapping--
evalProgList :: Prog -> [Pair String Int] -> Either String [Pair String Int]
evalProgList (Return res) l = Right l
evalProgList (Eq x expr) l = case evalExpr expr l of
                              Left msg -> Left msg
                              Right val -> Right (addToList (Cons x val) l)
evalProgList (Seq prog1 prog2) l = case evalProgList prog1 l of
                                    Left msg -> Left msg
                                    Right l1 ->
                                      case evalProgList prog2 l1 of
                                        Left msg -> Left msg
                                        Right l2 -> Right l2
evalProgList (If expr prog1 prog2) l = case evalExpr expr l of
                                        Left msg -> Left msg
                                        Right val -> if val == 1 then 
                                          case evalProgList prog1 l of
                                            Left msg -> Left msg
                                            Right l1 -> Right l1
                                            else
                                              case evalProgList prog2 l of
                                                Left msg -> Left msg
                                                Right l2 -> Right l2
evalProgList (While expr prog) l = case evalExpr expr l of
                                    Left msg -> Left msg
                                    Right val -> if val == 1 then 
                                      case evalProgList prog l of
                                        Left msg -> Left msg
                                        Right lProg -> (evalProgList (While expr prog) lProg)
                                      else Right l

--execute program using a key-value map as an accumulator--
evalProg :: Prog -> [Pair String Int] -> Either String Int
evalProg (Return res) l = case evalExpr res l of
                            Left msg -> Left msg
                            Right val -> Right val
evalProg (Eq x expr) l = case evalExpr expr l of
                          Left msg -> Left msg
                          Right val -> Left "OK"
evalProg (Seq (Eq x expr) prog) l = case evalExpr expr l of
                                      Left msg -> Left msg
                                      Right val -> evalProg prog (addToList (Cons x val) l)
evalProg (Seq prog1 prog2) l = case evalProg prog1 l of
                                  Left msg -> if msg == "Uninitialized variable" then Left msg else
                                    case evalProgList prog1 l of
                                      Left msg -> Left msg
                                      Right l2 -> 
                                        case evalProg prog2 l2 of
                                          Left msg -> Left msg
                                          Right val2 -> Right val2
                                  Right val1 -> Right val1
evalProg (If expr prog1 prog2) l = case evalExpr expr l of
                                    Left msg -> Left msg
                                    Right val ->
                                      if val == 1 then 
                                        case evalProg prog1 l of
                                          Left msg -> Left msg
                                          Right val1 -> Right val1
                                      else
                                        case evalProg prog2 l of
                                          Left msg -> Left msg
                                          Right val2 -> Right val2
evalProg (While expr prog) l = case evalExpr expr l of
                                Left msg -> Left msg
                                Right val ->
                                  if val == 1 then
                                    case evalProgList prog l of
                                      Left msg -> Left msg
                                      Right l2 -> evalProg (While expr prog) l2
                                  else
                                    Left "OK"

evalAdt :: Prog -> Either String Int
evalAdt prog = case evalProg prog [] of
                Left msg -> if msg /= "Uninitialized variable" then Left "Missing return" else Left msg
                Right val -> Right val

-- The *evalRaw* function is already implemented, but it relies on the *parse*
-- function which you have to implement.
--
-- Of couse, you can change this definition.  Only its name and type are
-- important.
evalRaw :: String -> Either String Int
evalRaw rawProg = case parse rawProg of
                    Just prog -> evalAdt prog
                    Nothing   -> Left "Syntax error"
