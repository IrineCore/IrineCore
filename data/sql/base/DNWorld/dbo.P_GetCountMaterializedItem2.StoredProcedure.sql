USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountMaterializedItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ?? ????? ?? ?? ??? ?? ?? ??? ??? ??? ????? ?? ???? ??????.
result set : RecordCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountMaterializedItem2]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 5=?? ????, 9=??, 10=Pet&??
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
	IF @inyItemLocationCode = 10
		SELECT COUNT(*) AS RecordCount
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode BETWEEN 9 AND 10;

	ELSE
		SELECT COUNT(*) AS RecordCount
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = @inyItemLocationCode;

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
