USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacterSlotCount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCharacterSlotCount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCharacterSlotCount] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-04
description : ??? ?? ?? ?????. (???? ???? ?? ?? ??? ?? ?????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = ???? ??     by ??? at 2012-03-16
7 = ?? ???? ??? ?? ?? ??? ? ??? ?? by ??? at 2012-04-30
**/

ALTER PROCEDURE [dbo].[P_GetCharacterSlotCount]
	  @inbCharacterID bigint --// ??? ID
	, @inyCharacterSlotCount tinyint OUTPUT --// ??? ?? ?
	, @intAccountID int = NULL --// ?? ID
	, @intWorldID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# Per Character Slot Mode - ??? ?? ??? ?? ?? ?? */
	IF @inbCharacterID > 0 OR (@intAccountID > 0 AND @intWorldID > 0)
	BEGIN
		IF @inbCharacterID > 0
			SELECT @intAccountID = AccountID, @intWorldID = WorldID
			FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
			WHERE CharacterID = @inbCharacterID
			OPTION (KEEPFIXED PLAN);

		SELECT @inyCharacterSlotCount = CharacterSlotCount
		FROM dbo.CharacterSlotCounts WITH (NOLOCK, INDEX(PK_CharacterSlotCounts), FORCESEEK)
		WHERE AccountID = @intAccountID AND WorldID = @intWorldID
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
			SELECT @inyCharacterSlotCount = CharacterMaxCount
			FROM dbo.Accounts WITH (NOLOCK, FORCESEEK, INDEX(PK_Accounts))
			WHERE AccountID = @intAccountID
			OPTION (KEEPFIXED PLAN);
	END

	/**_# Per Account Slot Mode - ??? ?? ??? ?? ?? ?? */
	ELSE
		SELECT @inyCharacterSlotCount = SUM(CASE WHEN ISNULL(SC.CharacterSlotCount, 0) > AC.CharacterMaxCount THEN (SC.CharacterSlotCount - AC.CharacterMaxCount) ELSE 0 END)
		FROM dbo.Accounts AC WITH (NOLOCK, INDEX(PK_Accounts), FORCESEEK)
			LEFT OUTER JOIN dbo.CharacterSlotCounts SC WITH (NOLOCK, INDEX(PK_CharacterSlotCounts), FORCESEEK) ON AC.AccountID = SC.AccountID
		WHERE AC.AccountID = @intAccountID
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
