# MongoDB dump/restore

Тази папка е за snapshot на базата `Eco_city_game`, така че всеки от екипа да може да си възстанови същата локална база без ръчно създаване на колекции и документи.

## Какво има тук

- `dump/`:
  Тук се държи export-ът на Mongo базата.
  След `mongodump` вътре трябва да се появи подпапка `Eco_city_game/` с `.bson` и `.metadata.json` файлове.

## Структура след dump

Очакваният резултат е подобен на:

```text
mongo/
  dump/
    Eco_city_game/
      games.bson
      games.metadata.json
      resources.bson
      resources.metadata.json
      current_petition.bson
      current_petition.metadata.json
      active_petitions.bson
      active_petitions.metadata.json
      petition_counts.bson
      petition_counts.metadata.json
```

## Как човек с работеща база прави dump

От проектната root папка:

```powershell
docker exec eco-city-mongo sh -c "rm -rf /backup/Eco_city_game && mongodump --db Eco_city_game --out /backup"
```

Това ще запише dump-а в `./mongo/dump/Eco_city_game`.

Ако контейнерът не е пуснат:

```powershell
docker compose up -d
```

## Как ce възстановява базата

1. Пуска Mongo контейнера:

```powershell
docker compose up -d
```

2. Възстановява dump-а:

```powershell
docker exec eco-city-mongo sh -c "mongorestore --drop --db Eco_city_game /backup/Eco_city_game"
```

## Полезни проверки

Вход в Mongo shell:

```powershell
docker exec -it eco-city-mongo mongosh
```

Проверка на базата:

```javascript
use Eco_city_game
show collections
db.games.find().pretty()
db.resources.find().pretty()
db.current_petition.find().pretty()
db.active_petitions.find().pretty()
db.petition_counts.find().pretty()
```

## Важно

- `docker-compose.yml` само стартира Mongo.
- За да има реални данни като при работещата ти локална база, трябва да има dump в `mongo/dump/Eco_city_game`.
- Ако колега restore-не dump-а, ще получи същата структура и документи, без ръчно създаване на колекции.
