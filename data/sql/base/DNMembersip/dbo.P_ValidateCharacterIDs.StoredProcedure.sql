USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ValidateCharacterIDs]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ValidateCharacterIDs]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ValidateCharacterIDs] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ??? ID? ???? ?????. (??? ?? 1,000? ?)
result set : ErrorType (1=??? ???, 2=?? ???, 3=WorldID? ???? ?? ???), WorldID, CharacterID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = input param. @intWorldID ?? at 2010-11-02
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_ValidateCharacterIDs]
	  @vchCharacterIDs varchar(max) --// ,? ?????? CharacterID ??? (?? 1,000?)
	, @intWorldID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblCharacterIDs table (CharacterID bigint NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblCharacterIDs (CharacterID)
	SELECT CAST(string AS bigint)
	FROM dbo.TF_Split(@vchCharacterIDs, ',')
	WHERE string > ''
	GROUP BY string
	ORDER BY string;

	SELECT CAST(1 AS tinyint) ErrorType, CH.WorldID, CH.CharacterID
	FROM @tblCharacterIDs T
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON T.CharacterID = CH.CharacterID
	WHERE CH.DeleteFlag = CAST(1 AS bit)
	UNION ALL
	SELECT CAST(2 AS tinyint), NULL, T.CharacterID
	FROM @tblCharacterIDs T
	WHERE NOT EXISTS (
		SELECT * FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = T.CharacterID
	)
	UNION ALL
	SELECT CAST(3 AS tinyint), CH.WorldID, CH.CharacterID
	FROM @tblCharacterIDs T
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON T.CharacterID = CH.CharacterID
	WHERE @intWorldID IS NOT NULL AND CH.DeleteFlag = CAST(0 AS bit) AND CH.WorldID <> @intWorldID
	ORDER BY ErrorType, WorldID, CharacterID;
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
