USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFairy]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-24
description : ??(???) ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103396 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModFairy]
	  @inbFairyItemSerial bigint --// ?? ??? ???
	, @inyFairyLevel tinyint = NULL --// ?? ??
	, @intFairyExp int = NULL --// ?? ???
	, @intFairyStatus1 int = NULL --// ????1
	, @intFairyStatus2 int = NULL --// ????2
	, @intFairyStatus3 int = NULL --// ????3
	, @intFairyStatus4 int = NULL --// ????4
	, @intFairyStatus5 int = NULL --// ????5
	, @intFairyStatus6 int = NULL --// ????6
	, @vchMaterialFairiesItemSerial varchar(max) = NULL --// ?? ???? ?? ??? ???? ??? ???(, ???)
	, @intShapeID tinyint = NULL --// ?? ShapeID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inyFairyLevelBefore tinyint
	  , @intChangedStatus1 int
	  , @intChangedStatus2 int
	  , @intChangedStatus3 int
	  , @intChangedStatus4 int
	  , @intChangedStatus5 int
	  , @intChangedStatus6 int
	  , @intCurrentStatus1 int
	  , @intCurrentStatus2 int
	  , @intCurrentStatus3 int
	  , @intCurrentStatus4 int
	  , @intCurrentStatus5 int
	  , @intCurrentStatus6 int	
	  , @i int = 1
	  , @j int
	  , @intItemID int
	  , @inbOwnerCharacterID bigint
	  , @dt2OwnershipStartDate datetime2(0)
	  , @insItemCount smallint
	  , @bitSoulBoundFlag bit
	  , @inySealCount tinyint
	  , @inyItemOption tinyint
	  , @inyItemPotential tinyint
	  , @insRemainItemCount smallint
	  , @insItemRemainCountBefore smallint
	  , @inyItemPotentialMoveCount tinyint
	  , @inyItemLevelBefore tinyint
	  , @inbMaterializedItemID bigint
	  , @inbCharacterID bigint
	  , @inbItemSerial bigint;

DECLARE @tblItemSerials table (
	ItemSerial bigint NOT NULL,
	SequenceNumber int NOT NULL
);

DECLARE @tblDeletedFairies table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ItemID int NOT NULL,
	OwnerCharacterID bigint NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL,
	ItemLevel tinyint NULL,
	MaterializedItemID bigint NOT NULL,
	ItemSerial bigint NOT NULL,	
	ItemRemainCount int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Fairies] ???? UPDATE???.*/
	UPDATE dbo.Fairies 
	SET @inyFairyLevelBefore = FairyLevel
	  , @intCurrentStatus1 = FairyStatus1
	  , @intCurrentStatus2 = FairyStatus2
	  , @intCurrentStatus3 = FairyStatus3
	  , @intCurrentStatus4 = FairyStatus4
	  , @intCurrentStatus5 = FairyStatus5
	  , @intCurrentStatus6 = FairyStatus6
	  , FairyLevel = COALESCE(@inyFairyLevel, CASE WHEN FairyLevel = 0 THEN FairyLevel ELSE @inyFairyLevel END)
	  , FairyExp = COALESCE(@intFairyExp, FairyExp)
	  , FairyStatus1 = COALESCE(@intFairyStatus1, FairyStatus1)
	  , FairyStatus2 = COALESCE(@intFairyStatus2, FairyStatus2)
	  , FairyStatus3 = COALESCE(@intFairyStatus3, FairyStatus3)
	  , FairyStatus4 = COALESCE(@intFairyStatus4, FairyStatus4)
	  , FairyStatus5 = COALESCE(@intFairyStatus5, FairyStatus5)
	  , FairyStatus6 = COALESCE(@intFairyStatus6, FairyStatus6)	  
	  , ShapeID = COALESCE(@intShapeID, ShapeID)	
	WHERE FairyItemSerial = @inbFairyItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103396;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?? ?? ?? ?? ???.*/
	IF @vchMaterialFairiesItemSerial IS NOT NULL
	BEGIN
		INSERT @tblItemSerials (ItemSerial, SequenceNumber)
		SELECT CAST (string AS bigint), CAST(seq AS int)
		FROM dbo.TF_Split(@vchMaterialFairiesItemSerial, ',');

		DELETE dbo.MaterializedItems
		OUTPUT deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemCount, deleted.SoulBoundFlag, deleted.SealCount
			 , deleted.ItemOption, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.ItemLevel, deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemRemainCount
		INTO @tblDeletedFairies (ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, ItemPotentialMoveCount, ItemLevel, MaterializedItemID, ItemSerial, ItemRemainCount)
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
			 INNER JOIN @tblItemSerials SI ON MI.ItemSerial = SI.ItemSerial;	
		
		SET @j = @@ROWCOUNT;

		IF @j > 0
		BEGIN
			WHILE @i <= @j
			BEGIN
				SELECT @intItemID = ItemID, @dt2OwnershipStartDate = OwnershipStartDate, @insItemCount = ItemCount
					, @bitSoulBoundFlag = SoulBoundFlag, @inySealCount = SealCount, @inyItemOption = ItemOption, @inyItemPotential = ItemPotential
					, @inbCharacterID = OwnerCharacterID, @inbItemSerial = ItemSerial
				FROM @tblDeletedFairies
				WHERE Seq = @i;

				EXEC dbo.P_AddItemLog
					  @dt2LogDate = @dt2Now
					, @inbItemSerial = @inbItemSerial
					, @intItemID = @intItemID
					, @inyItemLogCode = 80
					, @inbOwnerCharacterID = NULL
					, @inbOwnerCharacterIDBefore = @inbCharacterID
					, @insItemCount = 0
					, @insItemCountBefore = @insItemCount
					, @inbItemLogFKey1 = 0
					, @inbItemLogFKey2 = NULL
					, @dt2OwnershipStartDate = @dt2OwnershipStartDate
					, @intIP = NULL
					, @inySealCount = @inySealCount
					, @bitSoulBoundFlag = @bitSoulBoundFlag
					, @inyItemOption = @inyItemOption
					, @inyItemPotential = @inyItemPotential
					, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount						
					, @inyDragonJewelType = NULL;

				SET @i += 1;	
			END
		END			 

		DELETE dbo.Fairies
		FROM dbo.Fairies F WITH (NOLOCK)
			 INNER JOIN @tblItemSerials SI ON F.FairyItemSerial = SI.ItemSerial;

	END

	SET @intChangedStatus1 = CASE WHEN @intFairyStatus1 IS NULL THEN 0 ELSE @intFairyStatus1 - @intCurrentStatus1 END;
	SET @intChangedStatus2 = CASE WHEN @intFairyStatus2 IS NULL THEN 0 ELSE @intFairyStatus2 - @intCurrentStatus2 END;
	SET @intChangedStatus3 = CASE WHEN @intFairyStatus3 IS NULL THEN 0 ELSE @intFairyStatus3 - @intCurrentStatus3 END;
	SET @intChangedStatus4 = CASE WHEN @intFairyStatus4 IS NULL THEN 0 ELSE @intFairyStatus4 - @intCurrentStatus4 END;
	SET @intChangedStatus5 = CASE WHEN @intFairyStatus5 IS NULL THEN 0 ELSE @intFairyStatus5 - @intCurrentStatus5 END;
	SET @intChangedStatus6 = CASE WHEN @intFairyStatus6 IS NULL THEN 0 ELSE @intFairyStatus6 - @intCurrentStatus6 END;

	EXEC @intReturnValue = dbo.P_AddFairyLevelLog
		  @dt2LogDate = @dt2Now
		, @inbFairyItemSerial = @inbFairyItemSerial
		, @inyFairyLevelBefore = @inyFairyLevelBefore
		, @inyFairyLevelCurrent = @inyFairyLevel
		, @intChangedStatus1 = @intChangedStatus1
		, @intChangedStatus2 = @intChangedStatus2
		, @intChangedStatus3 = @intChangedStatus3
		, @intChangedStatus4 = @intChangedStatus4
		, @intChangedStatus5 = @intChangedStatus5
		, @intChangedStatus6 = @intChangedStatus6;

END TRY
BEGIN CATCH	
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
