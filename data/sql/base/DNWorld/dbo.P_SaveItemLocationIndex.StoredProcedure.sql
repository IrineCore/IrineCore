USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SaveItemLocationIndex]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 19
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ???? ?? ?? ??? ?????. (????-2, ??-3, ??? ????-4)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103203 = ??? ???? ???? ?????. ??? ? ????.
103151 = ???? ???? ????.
history :
6 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
7 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
8~9 = ?? ?? ????? ???? ??? ??, ?? ??? ??? UPDATE ??? ?? by ??? at 2012-07-18
10~11 = ???? ???? ?? UPDATE??? ??  by ??? at 2012-07-23
15 = ?? ?? by ??? at 2012-07-24
17 = ?? ?? by ??? at 2012-07-25
18 = ?? ?? by ??? at 2012-08-03
19 = 17???? ??? by ??? at 2012-08-10
**/

CREATE PROCEDURE [dbo].[P_SaveItemLocationIndex]
	  @inbCharacterID bigint --// ??? ID
	, @vchItemLocationIndexes varchar(8000) --// (???? & ??) ,? ?????? ??? ?? ??? ???
	, @vchItemSerials varchar(8000) --// (???? & ??) ,? ?????? ??? ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intIndex1 int = -1, @intIndex2 int = -1;

DECLARE @tblTemp1 table (
	ItemSerial bigint NOT NULL,
	ItemLocationIndex tinyint NOT NULL
);

DECLARE @tblTemp2 table (
	MaterializedItemID bigint NOT NULL,
	ItemLocationIndex tinyint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	WHILE (LEN(@vchItemSerials) > 0)
    BEGIN
		SET @intIndex1 = CHARINDEX(',', @vchItemSerials);
		SET @intIndex2 = CHARINDEX(',', @vchItemLocationIndexes);

		IF (@intIndex1 = 0) AND (LEN(@vchItemSerials) > 0)
		BEGIN
			INSERT @tblTemp1 (ItemSerial, ItemLocationIndex) VALUES (@vchItemSerials, @vchItemLocationIndexes);

			BREAK;
		END

		IF (@intIndex1 > 1)
		BEGIN
			INSERT @tblTemp1 (ItemSerial, ItemLocationIndex) VALUES (LEFT(@vchItemSerials, @intIndex1 - 1), LEFT(@vchItemLocationIndexes, @intIndex2 - 1));

			SET @vchItemSerials = RIGHT(@vchItemSerials, (LEN(@vchItemSerials) - @intIndex1));
			SET @vchItemLocationIndexes = RIGHT(@vchItemLocationIndexes, (LEN(@vchItemLocationIndexes) - @intIndex2));
		END
		ELSE
		BEGIN
			SET @vchItemSerials = RIGHT(@vchItemSerials, (LEN(@vchItemSerials) - @intIndex1));
			SET @vchItemLocationIndexes = RIGHT(@vchItemLocationIndexes, (LEN(@vchItemLocationIndexes) - @intIndex2));
		END
	END

	INSERT @tblTemp2 (MaterializedItemID, ItemLocationIndex)
	SELECT MI.MaterializedItemID, T.ItemLocationIndex
	FROM @tblTemp1 T
		INNER LOOP JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON MI.ItemSerial = T.ItemSerial
	WHERE MI.OwnerCharacterID = @inbCharacterID
		AND MI.ItemLocationCode BETWEEN 2 AND 4
		AND (MI.ItemLocationIndex IS NULL OR MI.ItemLocationIndex <> T.ItemLocationIndex)
	OPTION (KEEPFIXED PLAN);

	UPDATE dbo.MaterializedItems
	SET ItemLocationIndex = S.ItemLocationIndex
	FROM @tblTemp2 S
		INNER LOOP JOIN dbo.MaterializedItems T WITH (INDEX(PK_MaterializedItems), FORCESEEK) ON S.MaterializedItemID = T.MaterializedItemID AND T.ItemLocationCode BETWEEN 2 AND 4
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
