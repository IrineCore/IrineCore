USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_PutOnepieceOn2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ?????. (?? ???? -> ?? ????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103201 = ???? ??? ??? ???? ????.
history : 2
2 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_PutOnepieceOn2]
	  @inbCharacterID bigint --// ??? ID
	, @inbOnepieceItemSerial bigint --// ?? ????? ???? ??? ??? ???
	, @inyPrimaryCashEquipmentIndex tinyint --// ???? ???? Primary ?? ???? ???
	, @vchSubCashEquipmentIndexes varchar(50) --// ??? ?????? ?? ????? Sub ?? ???? ???? ???. ???? ,???.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbOnepieceMaterializedItemID bigint;
DECLARE @tblCashEquipmentIndexes table (ItemLocationIndex tinyint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ??? ???? ?? ?? ?? ??? ?????. ???? ???? ??? 103151? ?????.*/
	SELECT @inbOnepieceMaterializedItemID = MaterializedItemID
	FROM dbo.MaterializedItems WITH (NOLOCK) 
	WHERE ItemSerial = @inbOnepieceItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode = 5;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	/**_# ???? ??? ?? ????? ??? ???? ???, P_TakeCashEquipmentOff? ???? ????.*/
	INSERT @tblCashEquipmentIndexes (ItemLocationIndex)
	SELECT CAST(string AS tinyint) 
	FROM dbo.TF_Split(@vchSubCashEquipmentIndexes, ',') 
	WHERE string > ''
	UNION ALL
	SELECT @inyPrimaryCashEquipmentIndex
	OPTION (KEEPFIXED PLAN);

	IF EXISTS (
		SELECT * 
		FROM dbo.MaterializedItems MI WITH (NOLOCK)
			INNER JOIN @tblCashEquipmentIndexes T ON MI.ItemLocationIndex = T.ItemLocationIndex
		WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 7
	)
	BEGIN
		SET @vchSubCashEquipmentIndexes += (CASE @vchSubCashEquipmentIndexes WHEN '' THEN '' ELSE ',' END + CAST(@inyPrimaryCashEquipmentIndex AS varchar(3)));

		EXEC @intReturnValue = dbo.P_TakeCashEquipmentOff2
			  @inbCharacterID = @inbCharacterID
			, @vchCashEquipmentIndexes = @vchSubCashEquipmentIndexes;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	/**_# ???? ?? ????? ?????.*/
	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 7
		, ItemLocationIndex = @inyPrimaryCashEquipmentIndex
	WHERE MaterializedItemID = @inbOnepieceMaterializedItemID;
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
