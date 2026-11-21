USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCompleteQuestBinary]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ??? ??? ????? ?????.
result set : 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = @sdtLastQuestCheckDate output param. ?? at 2010-06-28
4 = CompleteQuestBin ?? ????? ???? ??? ?? at 2010-08-30
**/

CREATE PROCEDURE [dbo].[P_GetCompleteQuestBinary]
	  @inbCharacterID bigint --// ??? ID
	, @binCompleteQuestBin binary(1250) OUTPUT --// ?? ??? Binary
	, @sdtLastQuestCheckDate smalldatetime OUTPUT --// ??? ??? ?? ?? ??
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
	/**_# ??? ???? 1?? ?? SELECT ???.*/
	SELECT @binCompleteQuestBin = dbo.FN_CompleteQuestBin(@inbCharacterID), @sdtLastQuestCheckDate = LastQuestCheckDate
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
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
