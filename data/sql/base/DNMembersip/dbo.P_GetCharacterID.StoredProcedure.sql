USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCharacterID]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCharacterID] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-14
description : CharacterName?? CharacterID? ?? ?????. (??, ?? ?? ?? ???? ???? ????.)
result set : seq/int, CharacterName/nvarchar, CharacterID/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_GetCharacterID]
	  @nvcCharacterNames nvarchar(max) --// ,? ?????? ??? ?? ???
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
	SELECT T.seq, T.string AS CharacterName, CH.CharacterID
	FROM dbo.TF_NSplit(@nvcCharacterNames, N',') T
		LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON T.string = CH.CharacterName
	WHERE CH.DeleteFlag = CAST(0 AS bit)
	ORDER BY T.seq;
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
