USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ExchangeProperty2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???, ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103188 = ????? ???? ?? ???? ?? ????.
103292 = ???? ???? ?? ??????.
103304 = ?? ??? ??? ??? ? ????.
history :
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
4 = (44232) ??? ???? ItemLocationIndex ?? ????? ?? ?? ?? by ??? at 2011-11-03
5 = ??? ?? ??? ItemLevel(????)? ????? ?? by ??? at 2012-03-08
8 = ??? ??? Ownershipstartdate UPDATE ?? ?? ?? ?? by ??? at 2012-05-18
**/

CREATE PROCEDURE [dbo].[P_ExchangeProperty2]
	  @inbCharacterIDSideA bigint --// ??? ID
	, @vchItemSerialSideA varchar(8000) --// ,? ?????? ??? ??? ???
	, @vchItemIDSideA varchar(8000) --// ,? ?????? ??? ID ???
	, @vchNewItemSerialSideA varchar(8000) --// ,? ?????? ? ??? ??? ???. ?? ??? ??? ? ??? ? ??? ??????. ?? ?? 0?? ?????. ?) 0,0,0,3,0
	, @vchItemCountSideA varchar(8000) --// ,? ?????? ??? ??? ?? ???
	, @inbCoinSideA bigint --// ??? ??
	, @intExchangeCommissionA int --// A? ???? ?? ???

	, @inbCharacterIDSideB bigint --// ??? ID
	, @vchItemSerialSideB varchar(8000) --// ,? ?????? ??? ??? ???
	, @vchItemIDSideB varchar(8000) --// ,? ?????? ??? ID ???
	, @vchNewItemSerialSideB varchar(8000) --// ,? ?????? ? ??? ??? ???. ?? ??? ??? ? ??? ? ??? ??????. ?? ?? 0?? ?????. ?) 0,0,0,3,0
	, @vchItemCountSideB varchar(8000) --// ,? ?????? ??? ??? ?? ???
	, @inbCoinSideB bigint --// ??? ??
	, @intExchangeCommissionB int --// B? ???? ?? ???

	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @i int
	, @j int
	, @inbItemSerial bigint
	, @inbNewItemSerial bigint
	, @insItemCount smallint
	, @intLogTableSuffix int
	, @inbMaterializedItemID bigint
	, @inbNewMaterializedItemID bigint;

DECLARE @tblSideA table (
	seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	MaterializedItemID bigint NULL,
	NewMaterializedItemID bigint NULL,
	ItemSerial bigint NOT NULL,
	NewItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemCount smallint NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemRemainCount smallint NULL,
	OwnerCharacterID bigint NULL,
	ItemLocationCode tinyint NULL,
	ItemLevel tinyint NULL
);

DECLARE @tblSideB table (
	seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, 
	MaterializedItemID bigint NULL, 
	NewMaterializedItemID bigint NULL, 
	ItemSerial bigint NOT NULL, 
	NewItemSerial bigint NOT NULL, 
	ItemID int NOT NULL, 
	ItemCount smallint NOT NULL, 
	OwnershipStartDate datetime2(0) NULL, 
	ItemRemainCount smallint NULL, 
	OwnerCharacterID bigint NULL,
	ItemLocationCode tinyint NULL,
	ItemLevel tinyint NULL
);

DECLARE @tblTemp table (
	seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, 
	MaterializedItemID bigint NOT NULL, 
	NewItemSerial bigint NOT NULL, 
	ItemCount smallint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbCharacterIDSideA = @inbCharacterIDSideB
	BEGIN
		SET @intReturnValue = 103304;
		GOTO ErrorHandler;
	END

	/**_# Input Parameter ???*/
	INSERT @tblSideA (MaterializedItemID, ItemSerial, NewItemSerial, ItemID, ItemCount, OwnershipStartDate, ItemRemainCount, OwnerCharacterID, ItemLocationCode, ItemLevel)
	SELECT MI.MaterializedItemID
		, CAST(A.string AS bigint) AS ItemSerial
		, CASE B.string WHEN '0' THEN CAST(A.string AS bigint) ELSE CAST(B.string AS bigint) END AS NewItemSerial
		, CAST(C.string AS int) AS ItemID
		, CAST(D.string AS smallint) AS ItemCount
		, MI.OwnershipStartDate
		, MI.ItemRemainCount
		, MI.OwnerCharacterID
		, MI.ItemLocationCode
		, MI.ItemLevel
	FROM dbo.TF_Split(@vchItemSerialSideA, ',') A
		INNER JOIN dbo.TF_Split(@vchNewItemSerialSideA, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchItemIDSideA, ',') C ON A.seq = C.seq
		INNER JOIN dbo.TF_Split(@vchItemCountSideA, ',') D ON A.seq = D.seq
		LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON CAST(A.string AS bigint) = MI.ItemSerial
	WHERE A.string > '' AND B.string > '' AND C.string > '' AND D.string > ''
	OPTION (FORCE ORDER, KEEPFIXED PLAN);

	IF EXISTS (
		SELECT *
		FROM @tblSideA
		WHERE ItemCount = 0
			OR ItemCount > ItemRemainCount
			OR OwnerCharacterID IS NULL
			OR OwnerCharacterID <> @inbCharacterIDSideA
			OR ItemLocationCode <> 2
	)
	BEGIN
		SET @intReturnValue = 103188;
		GOTO ErrorHandler;
	END

	INSERT @tblSideB (MaterializedItemID, ItemSerial, NewItemSerial, ItemID, ItemCount, OwnershipStartDate, ItemRemainCount, OwnerCharacterID, ItemLocationCode, ItemLevel)
	SELECT MI.MaterializedItemID
		, CAST(A.string AS bigint)
		, CASE B.string WHEN '0' THEN CAST(A.string AS bigint) ELSE CAST(B.string AS bigint) END
		, CAST(C.string AS int)
		, CAST(D.string AS smallint)
		, MI.OwnershipStartDate
		, MI.ItemRemainCount
		, MI.OwnerCharacterID
		, MI.ItemLocationCode
		, MI.ItemLevel
	FROM dbo.TF_Split(@vchItemSerialSideB, ',') A
		INNER JOIN dbo.TF_Split(@vchNewItemSerialSideB, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchItemIDSideB, ',') C ON A.seq = C.seq
		INNER JOIN dbo.TF_Split(@vchItemCountSideB, ',') D ON A.seq = D.seq
		LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON CAST(A.string AS bigint) = MI.ItemSerial
	WHERE A.string > '' AND B.string > '' AND C.string > '' AND D.string > ''
	OPTION (FORCE ORDER, KEEPFIXED PLAN);

	IF EXISTS (
		SELECT *
		FROM @tblSideB
		WHERE ItemCount = 0
			OR ItemCount > ItemRemainCount
			OR OwnerCharacterID IS NULL
			OR OwnerCharacterID <> @inbCharacterIDSideB
			OR ItemLocationCode <> 2
	)
	BEGIN
		SET @intReturnValue = 103188;
		GOTO ErrorHandler;
	END

	INSERT @tblTemp (MaterializedItemID, NewItemSerial, ItemCount)
	SELECT MaterializedItemID, NewItemSerial, ItemCount
	FROM @tblSideA
	WHERE ItemSerial <> NewItemSerial
	UNION ALL
	SELECT MaterializedItemID, NewItemSerial, ItemCount
	FROM @tblSideB
	WHERE ItemSerial <> NewItemSerial
	OPTION (KEEPFIXED PLAN);

	SELECT @i = 1, @j = @@ROWCOUNT;

	BEGIN TRANSACTION;

	/**_# ??? ?????.*/
	IF @inbCoinSideA > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideB
			, @inyCoinChangeCode = 11
			, @inbCoinChangeFKey = @inbCharacterIDSideA
			, @inbCoin = @inbCoinSideA
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;

		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideA
			, @inyCoinChangeCode = 12
			, @inbCoinChangeFKey = @inbCharacterIDSideB
			, @inbCoin = @inbCoinSideA
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	IF @inbCoinSideB > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideA
			, @inyCoinChangeCode = 11
			, @inbCoinChangeFKey = @inbCharacterIDSideB
			, @inbCoin = @inbCoinSideB
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;

		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideB
			, @inyCoinChangeCode = 12
			, @inbCoinChangeFKey = @inbCharacterIDSideA
			, @inbCoin = @inbCoinSideB
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END
	/**_# ?? ???? ?????.*/
	IF @intExchangeCommissionA > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideA
			, @inyCoinChangeCode = 15
			, @inbCoinChangeFKey = NULL
			, @inbCoin = @intExchangeCommissionA
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END
	IF @intExchangeCommissionB > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterIDSideB
			, @inyCoinChangeCode = 15
			, @inbCoinChangeFKey = NULL
			, @inbCoin = @intExchangeCommissionB
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# ?? ??? ???? ???? Split???.*/
	WHILE @i <= @j
	BEGIN
		SELECT @inbMaterializedItemID = MaterializedItemID
			, @inbNewItemSerial = NewItemSerial
			, @insItemCount = ItemCount 
		FROM @tblTemp 
		WHERE seq = @i;

		EXEC @intReturnValue = dbo.P_SplitMaterializedItem
			  @dt2SplitDate = @dt2Now
			, @inbMaterializedItemID = @inbMaterializedItemID
			, @inbNewItemSerial = @inbNewItemSerial
			, @insItemCount = @insItemCount
			, @bitInitOwnerCharacter = 0
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inyCallProc = 2
			, @inbNewMaterializedItemID = @inbNewMaterializedItemID OUTPUT;

		UPDATE @tblSideA
		SET NewMaterializedItemID = @inbNewMaterializedItemID
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		UPDATE @tblSideB
		SET NewMaterializedItemID = @inbNewMaterializedItemID
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		SET @i += 1;
	END

	/**_# MaterializedItmes.OwnerCharacterID? UPDATE???.*/
	;WITH C1 (CharacterID, MaterializedItemID) AS (
		SELECT @inbCharacterIDSideB, COALESCE(T.NewMaterializedItemID, T.MaterializedItemID)
		FROM @tblSideA T
		UNION ALL
		SELECT @inbCharacterIDSideA, COALESCE(T.NewMaterializedItemID, T.MaterializedItemID)
		FROM @tblSideB T
	)
	UPDATE dbo.MaterializedItems
	SET OwnerCharacterID = C1.CharacterID
		, ItemLocationIndex = NULL
		, OwnershipStartDate = @dt2Now
	FROM dbo.MaterializedItems MI WITH (INDEX(PK_MaterializedItems),FORCESEEK)
		INNER JOIN C1 ON MI.MaterializedItemID = C1.MaterializedItemID
	OPTION (KEEPFIXED PLAN);

	/**_# ??? ??? ?????.*/
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel)
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideB, @inbCharacterIDSideA, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideA A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		UNION ALL
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideA, @inbCharacterIDSideB, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideB A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		OPTION (KEEPFIXED PLAN);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel)
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideB, @inbCharacterIDSideA, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideA A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		UNION ALL
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideA, @inbCharacterIDSideB, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideB A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		OPTION (KEEPFIXED PLAN);

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel)
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideB, @inbCharacterIDSideA, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideA A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		UNION ALL
		SELECT @dt2Now, NewItemSerial, ItemID, 29, @inbCharacterIDSideA, @inbCharacterIDSideB, ItemCount, ItemCount, NULL, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL, ItemLevel
		FROM @tblSideB A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 29))
		OPTION (KEEPFIXED PLAN);

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;
IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
