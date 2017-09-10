#!/usr/bin/env runhaskell

module Checker where

import System.Environment (getArgs)
import System.Directory (getDirectoryContents, createDirectoryIfMissing)
import System.FilePath ((</>), takeBaseName)

import Data.List (sortOn)
import Data.List.Utils (split, replace)
import Control.Monad (liftM, forM)

import Interpreter (Prog, evalRaw, evalAdt)

getFilesFrom :: FilePath -> IO [FilePath]
getFilesFrom topDir = liftM (map concatTop) . liftM filterHardLinks $ contents
  where
    concatTop = (</>) topDir
    filterHardLinks = filter (`notElem` [".", ".."])
    contents = getDirectoryContents topDir

inputToOut :: FilePath -> FilePath
inputToOut = replace "in" "out"

inputToRef :: FilePath -> FilePath
inputToRef = replace "adt_" "" . replace "raw_" "" . replaceInRef
  where
    replaceInRef = replace "in" "ref" . replace "input" "ref"

readInt :: String -> Int
readInt s = read s :: Int

getTestPoints :: FilePath -> Int
getTestPoints = readInt . head . split "." . caar . split "-" . takeBaseName
  where
    caar = head . tail

sortByIndex :: [String] -> [String]
sortByIndex = sortOn (readInt . head . split "-" . drop 4 . takeBaseName)

runTests :: String                        -- Identification string
         -> FilePath                      -- Input directory
         -> FilePath                      -- Output directory
         -> (String -> Either String Int) -- Evaluation function
         -> IO ()
runTests id inputs outputs eval = do
  putStrLn $ "Running " ++ id ++ " tests:"

  inputs <- sortByIndex `fmap` getFilesFrom inputs
  createDirectoryIfMissing False outputs

  points_total <- forM inputs $ \file -> do
    output <- eval `fmap` readFile file
    writeFile (inputToOut file) (show output)
    ref <- readFile (inputToRef file)

    let baseName = takeBaseName file
    let outputLine = baseName ++ " " ++ replicate (24 - length baseName) '.'

    if output == (read ref :: Either String Int)
       then putStrLn (outputLine ++ " OK") >> return (getTestPoints file)
       else putStrLn (outputLine ++ " FAIL") >> return 0

  putStrLn $ "\n\t\tTOTAL: "
             ++ show (sum points_total)
             ++ "/"
             ++ show (foldr1 (+) . map getTestPoints $ inputs)

main :: IO ()
main = do
  args <- getArgs
  let opt = head args

  if length args == 1 && (opt == "raw" || opt == "adt" || opt == "both")
    then if opt == "both" 
            then runTests "RAW" rawInputsDir rawOutputsDir evalRaw >>
                 putStrLn "" >>
                 runTests "ADT" adtInputsDir adtOutputsDir evalAdt'
            else if opt == "raw"
                   then runTests "RAW" rawInputsDir rawOutputsDir evalRaw
                   else runTests "ADT" adtInputsDir adtOutputsDir evalAdt'
    else putStrLn "Usage:\n\trunhaskell Checker.hs [raw|adt|both]"
  where
    evalAdt' fileContents = evalAdt (read fileContents :: Prog)

adtInputsDir = "./tests/adt_input"
adtOutputsDir = "./tests/adt_output"
rawInputsDir = "./tests/raw_input"
rawOutputsDir = "./tests/raw_output"
