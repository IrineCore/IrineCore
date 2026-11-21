USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModItemDurability]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 14
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-22
description : ??? ???? ?? ?????. (?? ??? ?? ??? ?? or ??? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
9 = ItemAttributeLogs ???? ??? ?? ??? ??? ??? ?????.
10 = JOIN UPDATE ??? ?? Dead Lock? ????? NOLOCK ?? ?? at 2010-12-09
11 = TF_Split? IF_Split?? ?? at 2010-12-21
12 = Tuning. Split ?? ??     by 20110615.wgkim
13 = ?? ?? by ??? at 2011-11-01
14 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_ModItemDurability]
	  @inbCharacterID bigint --// ??? ??? ??? ID
	, @vchItemSerials varchar(8000) --// ,? ?????? ??? ?? ?? ???
	, @vchItemDurabilities varchar(8000) --// ,? ?????? ??? ???
	, @inbCoin bigint --// ???? ???? ?? ?? ??.
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intLogTableSuffix int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @intIndex1 int
	  , @intIndex2 int
	  , @inbItemSerial bigint
	  , @insItemDurability smallint

--DECLARE @tblTemp1 table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL);
--DECLARE @tblTemp2 table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemDurability smallint NOT NULL);
DECLARE @tblItems table (ItemSerial bigint NOT NULL PRIMARY KEY, ItemDurability smallint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	WHILE (LEN(@vchItemSerials) > 0)
	BEGIN

		SELECT @intIndex1 = CHARINDEX(',', @vchItemSerials);
		SELECT @intIndex2 = CHARINDEX(',', @vchItemDurabilities);

		IF (@intIndex1 = 0) AND (LEN(@vchItemSerials) > 0)
		BEGIN
			SELECT @inbItemSerial = @vchItemSerials;
			SELECT @insItemDurability = @vchItemDurabilities;

			INSERT @tblItems (ItemSerial, ItemDurability)
			SELECT @inbItemSerial, @insItemDurability;

			BREAK;
		END

		IF (@intIndex1 > 1)
		BEGIN
			SELECT @inbItemSerial = LEFT(@vchItemSerials, @intIndex1 - 1);
			SELECT @insItemDurability = LEFT(@vchItemDurabilities, @intIndex2 - 1);

			INSERT @tblItems (ItemSerial, ItemDurability)
			SELECT @inbItemSerial, @insItemDurability;

			SELECT @vchItemSerials = RIGHT(@vchItemSerials, LEN(@vchItemSerials) - @intIndex1);
			SELECT @vchItemDurabilities = RIGHT(@vchItemDurabilities, LEN(@vchItemDurabilities) - @intIndex2);
		END
		ELSE
		BEGIN
			SELECT @vchItemSerials = RIGHT(@vchItemSerials, LEN(@vchItemSerials) - @intIndex1);
			SELECT @vchItemDurabilities = RIGHT(@vchItemDurabilities, LEN(@vchItemDurabilities) - @intIndex2);
		END

	END

	/**_# [MaterializedItems] ???? UPDATE???.*/
	UPDATE dbo.MaterializedItems
	SET ItemDurability = T.ItemDurability
	FROM @tblItems T
		INNER JOIN dbo.MaterializedItems MI WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON MI.ItemSerial = T.ItemSerial
	OPTION (KEEPFIXED PLAN);

	/**_# ???? ??? ?? ?? ??? ?????.*/
	IF @inbCoin > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = 14
			, @inbCoinChangeFKey = NULL
			, @inbCoin = @inbCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;
	END
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
