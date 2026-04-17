import sys
from pymongo import MongoClient

MONGO_URI = "mongodb://localhost:27017/"
DB_NAME   = "Eco_city_game"
GAME_ID   = sys.argv[1] if len(sys.argv) > 1 else "local_game"

client = MongoClient(MONGO_URI)
db = client[DB_NAME]
filt = {"game_id": GAME_ID}

collections = [
    "games",
    "resources",
    "current_petition",
    "active_petitions",
    "petition_counts"
]

# Check existence
game = db["games"].find_one(filt)
if game is None:
    print(f"No save found for game_id='{GAME_ID}'")
    sys.exit(1)

print(f"About to DELETE all data for game_id = '{GAME_ID}'")
print(f"This will affect collections: {', '.join(collections)}")

confirm = input("Type 'DELETE' to confirm: ")
if confirm != "DELETE":
    print("Aborted.")
    sys.exit(0)

# Delete from all collections
total_deleted = 0

for col in collections:
    result = db[col].delete_many(filt)
    print(f"{col}: deleted {result.deleted_count} document(s)")
    total_deleted += result.deleted_count

print("\n" + "=" * 50)
print(f"Done. Total documents deleted: {total_deleted}")
print("=" * 50)
