USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetStageRewardLogs]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-11
description : ???? ?? ??? ?????
return value : StageStartDate/datetime, StageEndDate/datetime, CharacterID/bigint, RewardBoxCode/tinyint, RewardItemSerial/tinyint, RewardItemID/int, WorldID/int, RewardFailReason/int
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history:
2 = ????
3 = Parameter ??	20111013.ohtack
4 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
6 = WorldID ??? DimCharacters ???? ?? by ??? at 2012-05-15
**/

CREATE PROCEDURE [dbo].[P_GetStageRewardLogs]
	  @inbStageStartID bigint --// ???? ?? ID
	, @inbCharacterID bigint --// ??? ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'StageRewardLogs'
	, @nvcTblName2 nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, GETDATE()), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -1, GETDATE()), 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName1
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT1 OUTPUT;	
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;		
		
	
	/**_# [StageRewardLogs] ???? ?????.*/
	SET @nvcStmt = N'
	SELECT RL.StageStartDate, RL.StageEndDate, RL.CharacterID, RL.RewardBoxCode, RL.RewardItemSerial, RL.RewardItemID, DC.WorldID, RL.RewardFailReason
      FROM dbo.DimCharacters DC WITH(NOLOCK) 
			INNER JOIN (' + @nvcStmtOUT1 + N') RL ON DC.CharacterID = RL.CharacterID 
			INNER JOIN (' + @nvcStmtOUT2 + N') EL ON RL.StageStartID = EL.StageStartID
     WHERE RL.StageStartID = @inbStageStartID
       AND RL.CharacterID = @inbCharacterID
	';
	EXEC sp_executesql @nvcStmt, N'@inbStageStartID bigint, @inbCharacterID bigint', @inbStageStartID = @inbStageStartID, @inbCharacterID = @inbCharacterID;
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
