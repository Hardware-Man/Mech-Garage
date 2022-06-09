open Proj2_types;;

let getStartSymbol (g : grammar) : string =
  match g with (a,_) -> a
;;

let getNonterminals (g : grammar) : string list =
  let rec getNonTerm (s: (string * string list) list) : string list =
    match s with
      [] -> []
      | (q::r) -> (match q with (c,_) -> c::(getNonTerm r))
  in
  match g with (_,b) -> 
    match b with
      [] -> []
      | (h::t) -> 
        (getNonTerm (h::t))
;;

let getInitFirstSets (g : grammar) : symbolMap =
  List.fold_left (fun m t -> SMap.add (fst t) (snd t) m) SMap.empty (List.map (fun x -> (x,SymbolSet.empty)) (getNonterminals g))
;;

let getInitFollowSets (g : grammar) : symbolMap =
  SMap.add (getStartSymbol g) (SymbolSet.singleton "eof") (getInitFirstSets g)
;;

let rec computeFirstSet (first : symbolMap) (symbolSeq : string list) : SymbolSet.t =
  match symbolSeq with
    [] -> SymbolSet.empty
    | (h::t) ->
      if (SMap.mem h first)
        then 
          (
            let bset = SMap.find h first in
            if (SymbolSet.mem "eps" bset)
              then 
                (
                  if (t = [])
                    then (bset)
                  else 
                    (
                      let cset = computeFirstSet first t in
                      if (SymbolSet.is_empty cset)
                        then (bset)
                      else (SymbolSet.union (SymbolSet.remove "eps" bset) cset)
                    )
                )
            else (bset)
          )
      else (SymbolSet.singleton h)
;;

let recurseFirstSets (g : grammar) (first : symbolMap) firstFunc : symbolMap =
  match g with (_,b) ->
    match b with
      [] -> SMap.empty
      | (h::t) ->
        List.fold_left 
          (
            fun w x -> match x with ((y,z) : (string * string list)) -> 
              if (z = []) 
                then (SMap.add y (SymbolSet.union (SMap.find y w) (firstFunc first ("eps"::[]))) w)
              else (SMap.add y (SymbolSet.union (SMap.find y w) (firstFunc first z)) w)
          )
          first
          b
;;

let rec getFirstSets (g : grammar) (first : symbolMap) firstFunc : symbolMap =
  let basemap = recurseFirstSets g first firstFunc in
  if (SMap.equal SymbolSet.equal first basemap)
    then (basemap)
  else (getFirstSets g basemap firstFunc)
;;

let rec updateFollowSet (first : symbolMap) (follow : symbolMap) (nt : string) (symbolSeq : string list) : symbolMap =
  match symbolSeq with
    [] -> follow
    | (h::t) ->
      let upset = updateFollowSet first follow nt t in
      if (t != [])
        then
        (
          if (SMap.mem h first)
            then 
              (
                let fset = computeFirstSet first t in
                let bimap = SMap.find h follow in
                if (SymbolSet.mem "eps" fset)
                  then (SMap.add h (SymbolSet.union bimap (SymbolSet.union (SymbolSet.remove "eps" fset) (SMap.find nt follow))) upset)
                else (SMap.add h (SymbolSet.union bimap fset) upset)
              )
          else (upset)
        )
      else follow
;;

let recurseFollowSets (g : grammar) (first : symbolMap) (follow : symbolMap) followFunc : symbolMap =
  match g with (_,b) ->
    match b with
      [] -> SMap.empty
      | (h::t) ->
        List.fold_left
          (
            fun w x -> match x with ((y,z) : (string * string list)) ->
              if(z = [])
                then (SMap.fold (fun k d a -> SMap.add k (SymbolSet.union d (SMap.find k a)) a) (followFunc first w y ("eps"::[])) w)
              else
                (
                  let bkmap = (SMap.fold (fun k d a -> SMap.add k (SymbolSet.union d (SMap.find k a)) a) (followFunc first w y z) w) in
                  let blast = (List.nth z ((List.length z) - 1)) in
                  if (SMap.mem blast first)
                    then (SMap.add blast (SymbolSet.union (SMap.find blast bkmap) (SMap.find y bkmap)) bkmap)
                  else (bkmap)
                )
          )
          follow
          b
;;

let rec getFollowSets (g : grammar) (first : symbolMap) (follow : symbolMap) followFunc : symbolMap =
  let basemap = recurseFollowSets g first follow followFunc in
  if (SMap.equal SymbolSet.equal follow basemap)
    then (basemap)
  else (getFollowSets g first basemap followFunc)
;;

let rec getPredictSets (g : grammar) (first : symbolMap) (follow : symbolMap) firstFunc : ((string * string list) * SymbolSet.t) list =
  match g with (a,b) ->
    match b with
      [] -> []
      | (h::t) ->
        match h with (x,y) ->
          let fdelta = 
            if (y = [])
              then (firstFunc first ("eps"::[]))
            else (firstFunc first y)
          in
          if (SymbolSet.mem "eps" fdelta)
            then ((h, (SymbolSet.union (SymbolSet.remove "eps" fdelta) (SMap.find x follow)))::(getPredictSets (a,t) first follow firstFunc))
          else ((h, fdelta)::(getPredictSets (a,t) first follow firstFunc))
;;

let tryDerive (g : grammar) (inputStr : string list) : bool =
  let startgram = ((getStartSymbol g)::("eof"::[])) in
  let iFirSets = (getInitFirstSets g) in 
  let gFirSets = (getFirstSets g iFirSets computeFirstSet) in
  let iFolSets = (getInitFollowSets g) in
  let gFolSets = (getFollowSets g gFirSets iFolSets updateFollowSet) in
  let preSetList = (getPredictSets g gFirSets gFolSets computeFirstSet) in
  let rec mcheck (ins :string list) (sg : string list) : bool =
    match ins, sg with 
      (h::t), (q::r) ->
          if (SMap.mem q gFirSets)
            then
              (
                let rlcheck =
                  List.fold_left
                    (
                      fun x y ->
                        match y with ((a,b),z) ->
                          if (String.equal a q)
                            then
                              (
                                if (SymbolSet.mem h z)
                                  then (b@x)
                                else (x)
                              )
                            else (x)
                    )
                    []
                    preSetList
                in
                mcheck ins (rlcheck@r)
              )
          else
            (
              if (String.equal h q)
                then (mcheck t r)
              else (false)
            )
      | [], [] -> true
      | (j::k), [] -> false
      | [], (m::n) -> false
  in
  (mcheck (inputStr@("eof"::[])) startgram) 
;;

let tryDeriveTree (g : grammar) (inputStr : string list) : parseTree =
  let startgram = ((getStartSymbol g)::("eof"::[])) in
  let iFirSets = (getInitFirstSets g) in 
  let gFirSets = (getFirstSets g iFirSets computeFirstSet) in
  let iFolSets = (getInitFollowSets g) in
  let gFolSets = (getFollowSets g gFirSets iFolSets updateFollowSet) in
  let preSetList = (getPredictSets g gFirSets gFolSets computeFirstSet) in
  let rec mcheck (ins : string list) (sg : string list) : (parseTree * string list) =
    match ins, sg with
      [],[] -> (Terminal "empty",[])
      | [],(h::t) -> (Nonterminal (h,[]),ins)
      | (h::t),[] -> (Terminal "empty",[])
      | (h::t),(q::r) ->
        if (SMap.mem q iFirSets)
          then
            (
              let rlcheck =
                List.fold_left
                  (
                    fun x y ->
                      match y with ((a,b),z) ->
                        if (String.equal a q)
                          then
                            (
                              if (SymbolSet.mem h z)
                                then (b@x)
                              else (x)
                            )
                          else (x)
                  )
                  []
                  preSetList
              in
              List.fold_left
                (
                  fun v w ->
                    match v with (c,d) -> match c with Nonterminal(gg,ez) ->
                      let tempvar = (mcheck d (w::[])) in
                      match tempvar with (gl,hf) ->
                        match gl with 
                          Terminal wut ->
                            if(String.equal wut "eof")
                              then
                                (Nonterminal(gg,ez),hf)
                              else
                                (
                                  let lol = (ez@(gl::[])) in
                                  (Nonterminal(gg,lol),hf)
                                )
                          | Nonterminal(_,_) ->
                            let lol = (ez@(gl::[])) in
                            (Nonterminal(gg,lol),hf)
                )
                (Nonterminal(q,[]),ins)
                rlcheck
            )
        else (Terminal q,t)
  in
  match (mcheck (inputStr@("eof"::[])) startgram) with (pst,_) -> pst
;;

let genParser g = tryDerive g;;
let genTreeParser g = tryDeriveTree g;;
