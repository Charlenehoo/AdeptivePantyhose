HighHeel 持有 MorphName Heel=0
Pantyhose 持有 MorphName Heel=0 NoHeel=0

RE::BGSBipedObjectForm::BipedObjectSlot::kFeet 有 3 种状态:

- Barefeet
- LowHeel
- HighHeel

RE::BGSBipedObjectForm::BipedObjectSlot::kCalves 有 2 种状态:

- Barelegs
- Pantyhose

HighHeel + TipToe = Seamless
HighHeel + FlatFeet = Seam
FlatShoe + TipToe = Seam FlatShoe + FlatFeet = Seamless Barefoot+ TipToe = Seam Barefoot+ FlatFeet = Seamless
