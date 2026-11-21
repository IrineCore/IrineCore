USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModCharacterSlotCount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModCharacterSlotCount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModCharacterSlotCount] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-04
description : ??? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_ModCharacterSlotCount]
	  @inbCharacterID int --// ??? ID
	, @inyIncreasedCharacterSlotCount tinyint --// ??? ?? ?? ?
	, @inyCharacterSlotCount tinyint OUTPUT --// ?? ? ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inyDefaultCharacterSlotCount tinyint, @intAccountID int, @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID, @intWorldID = WorldID
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	/**_# [CharacterSlotCounts] ???? UPSERT???.*/
	UPDATE dbo.CharacterSlotCounts
	SET @inyCharacterSlotCount = CharacterSlotCount += @inyIncreasedCharacterSlotCount
	WHERE AccountID = @intAccountID AND WorldID = @intWorldID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SELECT @inyDefaultCharacterSlotCount = CharacterMaxCount
		FROM dbo.Accounts WITH (NOLOCK)
		WHERE AccountID = @intAccountID
		OPTION (KEEPFIXED PLAN);

		INSERT dbo.CharacterSlotCounts (AccountID, WorldID, CharacterSlotCount)
		VALUES (@intAccountID, @intWorldID, @inyDefaultCharacterSlotCount + @inyIncreasedCharacterSlotCount)
		OPTION (KEEPFIXED PLAN);

		SET @inyCharacterSlotCount = @inyDefaultCharacterSlotCount + @inyIncreasedCharacterSlotCount;
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
