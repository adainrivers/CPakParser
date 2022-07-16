#pragma once

#include "IOStore.h"

class FFileIoStore final
{
public:
	FIoContainerHeader Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
	void Initialize();

	class Reader
	{
	public:
		Reader(const char* InTocFilePath);
		~Reader();

		uint32_t GetContainerInstanceId() const
		{
			return ContainerFile.ContainerInstanceId;
		}
		
		std::shared_ptr<FIoStoreTocResource> GetTocResource() { return ContainerFile.TocResource; };
		const FFileIoStoreContainerFile& GetContainerFile() const { return ContainerFile; }
		const FIoContainerId& GetContainerId() const { return ContainerFile.TocResource->Header.ContainerId; }
		__forceinline bool IsEncrypted() const { return ContainerFile.TocResource->Header.IsEncrypted(); }
		bool IsSigned() const { return EnumHasAnyFlags(ContainerFile.TocResource->Header.ContainerFlags, EIoContainerFlags::Signed); }
		const FGuid& GetEncryptionKeyGuid() const { return ContainerFile.TocResource->Header.EncryptionKeyGuid; }
		void SetEncryptionKey(const FAESKey& Key) { ContainerFile.EncryptionKey = Key; }
		const FAESKey& GetEncryptionKey() const { return ContainerFile.EncryptionKey; }
		FIoContainerHeader ReadContainerHeader();

	private:
		FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
		uint64_t GetTocAllocatedSize() const;

		phmap::flat_hash_map<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
		FFileIoStoreContainerFile ContainerFile;
		bool bClosed = false;
		bool bHasPerfectHashMap = false;

		static std::atomic_uint32_t GlobalPartitionIndex;
		static std::atomic_uint32_t GlobalContainerInstanceId;
	};

private:
	uint64_t ReadBufferSize = 0;

	mutable std::shared_mutex IoStoreReadersLock;
	std::vector<std::unique_ptr<Reader>> IoStoreReaders;
};

class FIoStoreToc //TODO: revisit this and see if these index maps are necessary 
{
public:

	FIoStoreToc() //never use this
	{

	}

	FIoStoreToc(FIoStoreTocResource& TocRsrc) : Toc(std::make_shared<FIoStoreTocResource>(TocRsrc))
	{
		Initialize();
	}

	FIoStoreToc(std::shared_ptr<FIoStoreTocResource> TocRsrc) : Toc(TocRsrc)
	{
		Initialize();
	}

	FAESKey& GetEncryptionKey()
	{
		return Key;
	}

private:

	__forceinline void Initialize();

	FAESKey Key;
	std::shared_ptr<FIoStoreTocResource> Toc;
};

class FIoStoreReader
{
public:

	FIoStoreReader(const char* ContainerPath)
	{
		auto TocRsrc = FIoStoreTocResource(ContainerPath, EIoStoreTocReadOptions::ReadAll);
		this->Initialize(std::make_shared<FIoStoreTocResource>(TocRsrc));
	}

	FIoStoreReader(FIoStoreTocResource& TocResource)
	{
		this->Initialize(std::make_shared<FIoStoreTocResource>(TocResource));
	}

	FIoStoreReader(std::shared_ptr<FIoStoreTocResource> TocResourcePtr)
	{
		this->Initialize(TocResourcePtr);
	}

private:
	void Initialize(std::shared_ptr<FIoStoreTocResource> TocResource);

	FIoStoreToc Toc;
};