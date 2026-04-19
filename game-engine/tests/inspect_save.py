import sys
from pymongo import MongoClient

MONGO_URI   = "mongodb://localhost:27017/"
DB_NAME     = "Eco_city_game"
GAME_ID     = sys.argv[1] if len(sys.argv) > 1 else "local_game"

client = MongoClient(MONGO_URI)
db     = client[DB_NAME]
filt   = {"game_id": GAME_ID}

# Check the game exists
game = db["games"].find_one(filt)
if game is None:
    print(f"No save found for game_id='{GAME_ID}'")
    sys.exit(1)

print(f"{'='*60}")
print(f"  Save for game_id = {GAME_ID}")
print(f"{'='*60}")

# Resources
resources = list(db["resources"].find(filt, {"_id": 0}))
print("\nResources:")
if resources:
    col_w = 28
    print(f"  {'Type':<{col_w}} {'Amount':>15} {'Delta/tick':>12}")
    print(f"  {'-'*col_w} {'-'*15} {'-'*12}")
    for r in resources:
        print(f"  {r['type']:<{col_w}} {r['amount']:>15,.0f} {r['changes_per_tick']:>+12,.0f}")
else:
    print("  (none)")

# ── Current petition ──────────────────────────────────────────────────────────
print("\nCurrent Petition (shown to player):")
current = db["current_petition"].find_one(filt, {"_id": 0})
if current:
    print(f"  ID            : {current.get('petition_id')}")
    print(f"  Building      : {current.get('type')}")
    print(f"  Cost          : {current.get('cost'):,.0f} × 100k GBP")
    print(f"  Ticks to build: {current.get('ticks_needed_for_construction')}")
    print(f"  Effects per tick:")
    for key in ["water_per_tick_change","energy_per_tick_change","money_per_tick_change",
                "population_per_tick_change","co2_per_tick_change"]:
        val = current.get(key, 0)
        if val != 0:
            label = key.replace("_per_tick_change", "").upper()
            print(f"    {label:<12}: {val:+,.0f}")
else:
    print("  (none)")

# ── Under-construction petitions ──────────────────────────────────────────────
active = list(db["active_petitions"].find(filt, {"_id": 0}))
print(f"\nUnder Construction ({len(active)}):")
if active:
    for p in active:
        print(f"  [{p.get('petition_id')}] {p.get('type')}  "
              f"– {p.get('ticks_needed_for_construction')} tick(s) remaining")
else:
    print("  (none)")

# ── Building counts ───────────────────────────────────────────────────────────
counts = list(db["petition_counts"].find(filt, {"_id": 0}))
print(f"\nCompleted Buildings:")
if counts:
    for c in counts:
        print(f"  {c['type']:<40} ×{c['count']}")
else:
    print("  (none)")

print(f"\n{'='*60}")
print(f"Collections queried: games, resources, current_petition,")
print(f"                     active_petitions, petition_counts")
print(f"{'='*60}\n")
