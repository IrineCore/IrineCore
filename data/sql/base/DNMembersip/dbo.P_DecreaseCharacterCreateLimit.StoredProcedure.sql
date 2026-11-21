USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DecreaseCharacterCreateLimit]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DecreaseCharacterCreateLimit]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DecreaseCharacterCreateLimit] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ?? ?? ??? ???? ?? 1 ?? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101114 = ??? ?? ??? ???????. ??? ???? ? ?? ??? ? ????.
**/

ALTER PROCEDURE [dbo].[P_DecreaseCharacterCreateLimit]
	  @intAccountID int --// ??? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inyCharacterCreateLimit tinyint
	, @datLastCharacterCreateDate date;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Accounts] ???? UPDATE ???.*/
	UPDATE dbo.Accounts
	SET @inyCharacterCreateLimit = CharacterCreateLimit, @datLastCharacterCreateDate = LastCharacterCreateDate 
		, CharacterCreateLimit = 
			CASE
				WHEN LastCharacterCreateDate IS NULL OR LastCharacterCreateDate < CAST(GETDATE() AS date) THEN 6
				ELSE CASE CharacterCreateLimit WHEN 0 THEN 0 ELSE CharacterCreateLimit - 1 END
			END
		, LastCharacterCreateDate = 
			CASE
				WHEN LastCharacterCreateDate IS NULL OR LastCharacterCreateDate < CAST(GETDATE() AS date) OR CharacterCreateLimit > 0 THEN GETDATE()
				ELSE LastCharacterCreateDate
			END
	WHERE AccountID = @intAccountID;

	/**_# CharacterCreateLimit ?? ?? 0?? 101114? ?????.*/
	IF @datLastCharacterCreateDate >= CAST(GETDATE() AS date) AND @inyCharacterCreateLimit = 0
	BEGIN
		SET @intReturnValue = 101114;
		GOTO ErrorHandler;
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
