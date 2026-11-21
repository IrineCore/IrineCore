USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRepresentativeNewbieCharacterLevelDistribution]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-10
description : ?? ??? ?? ??? ?? ??? ?????.
result set : CharacterLevel, CharacterCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListRepresentativeNewbieCharacterLevelDistribution]
	  @datBasisDate date --// ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	SELECT CharacterLevel, SUM(CharacterCount) AS CharacterCount
	FROM dbo.ST_JobAndLevelType5 WITH (NOLOCK)
	WHERE BasisDate = @datBasisDate
	GROUP BY CharacterLevel
	ORDER BY CharacterLevel;
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
