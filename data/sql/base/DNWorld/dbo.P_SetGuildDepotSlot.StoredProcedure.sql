USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetGuildDepotSlot]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-08
description : ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

CREATE PROCEDURE [dbo].[P_SetGuildDepotSlot]
	  @intGuildID int --// ?? ID
	, @vchItemSerials varchar(max) --// ,? ?????? ??? ?? ?? ???
	, @vchGuildDepotIndexes varchar(max) --// ,? ?????? ???? ??? ?? ?? ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @tblSource table (
	ItemSerial bigint NOT NULL PRIMARY KEY,
	GuildID int NOT NULL,
	GuildDepotIndex smallint NOT NULL
);


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblSource (GuildID, ItemSerial, GuildDepotIndex)
	SELECT @intGuildID, CAST(A.string AS bigint), CAST(B.string AS smallint)
	FROM dbo.TF_Split(@vchItemSerials, ',') A
		INNER JOIN dbo.TF_Split(@vchGuildDepotIndexes, ',') B ON A.seq = B.seq
	OPTION (KEEPFIXED PLAN);

	/**_# [GuildDepotSlots] ???? MERGE???.*/
	WITH T (GuildID, ItemSerial, GuildDepotIndex) AS (
		SELECT GuildID, ItemSerial, GuildDepotIndex
		FROM dbo.GuildDepotSlots WITH (UPDLOCK)
		WHERE GuildID = @intGuildID
	)
	MERGE T
	USING @tblSource S
	ON T.GuildID = S.GuildID AND T.ItemSerial = S.ItemSerial
	WHEN MATCHED AND T.GuildDepotIndex <> S.GuildDepotIndex THEN
		UPDATE SET GuildDepotIndex = S.GuildDepotIndex
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (GuildID, ItemSerial, GuildDepotIndex) VALUES (S.GuildID, S.ItemSerial, S.GuildDepotIndex)
	WHEN NOT MATCHED BY SOURCE THEN
		DELETE;
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
