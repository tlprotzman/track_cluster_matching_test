from rucio.client import Client
rucio_client = Client()

datasets = rucio_client.list_dids("epic", [{"type" : "DATASET", "name" : "/RECO/25.07.0/epic_craterlake/SINGLE/e-/*"}])
print("Found the following datasets:")
ds_to_use = []
for dataset in datasets:
    print(dataset)
    if "etaScan" not in dataset:
        ds_to_use.append(dataset)

# Get the files for each dataset
ds_files = {}
for dataset in ds_to_use:
    files = rucio_client.list_dids("epic", [{"type" : "FILE", "name": f"{dataset}/*"}])
    ds_files[dataset] = []
    for file in files:
        ds_files[dataset].append(file)


total_files = 0
print("\nUsing the following datasets:")
for dataset, files in ds_files.items():
    print(f"Dataset: {dataset}\tSize: {len(files)}")
    total_files += len(files)

print("\nTruncating each dataset to 10 files")
for dataset in ds_files:
    ds_files[dataset] = ds_files[dataset][:10]

for dataset, files in ds_files.items():
    print(f"Dataset: {dataset}\tSize: {len(files)}")

print("Writing files list to files.list...")
with open("files.list", "w") as f:
    for dataset, files in ds_files.items():
        for file in files:
            f.write(f"{file}\n")
print("Done!")

