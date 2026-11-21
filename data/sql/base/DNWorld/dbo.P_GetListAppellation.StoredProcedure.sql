USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAppellation]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ?? ??? ?????.
result set : 
	version 2 = AppellationID/int, ChoiceFlag/bit, CoverChoiceFlag/bit
	version 3 = AppellationID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103196 = ??? ??? ????.
history :
2 = ?? ??? CoverChoiceFlag ?? at 2011-01-13
3 = (59407) intput param. @intVersion ?? by ??? at 2012-06-12
**/

CREATE PROCEDURE [dbo].[P_GetListAppellation]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 2
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
	IF @intVersion = 2
		SELECT AppellationID, ChoiceFlag, CoverChoiceFlag
		FROM dbo.Appellations WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

	ELSE IF @intVersion = 3
		SELECT AppellationID
		FROM dbo.Appellations WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103196;
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
