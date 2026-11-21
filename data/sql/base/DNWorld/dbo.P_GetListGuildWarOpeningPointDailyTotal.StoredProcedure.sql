USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPointDailyTotal]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??? ??? ?? ?? (??? 1?? ??)
result set :
    BasisDate	date
    OpeningPointSum	int
    CharacterID	bigint
    CharacterName	nvarchar(30)
    GuildID	int
    GuildName	nvarchar(50)
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    5 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPointDailyTotal]
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

	/**_# ??? ?? ??? ??? ?? ?? 1?? ?????. */
    SELECT PT.BasisDate
         , PT.OpeningPointSum
         , C.CharacterID
         , C.CharacterName
         , G.GuildID
         , G.GuildName
		 , ROW_NUMBER() OVER(PARTITION BY PT.BasisDate ORDER BY C.CharacterName) AS SortNumber
	  INTO #Temp_Result
      FROM dbo.GuildWarOpeningPointDailyTotal PT WITH (NOLOCK)
           INNER JOIN dbo.Characters C WITH (NOLOCK) ON (PT.CharacterID = C.CharacterID)
           INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
     WHERE PT.Ranking = 1
     ORDER BY PT.BasisDate;

	/**_## ?? 1?? ???? ???? ?????? ???? ?? ?? ???? 1?? ?????. */
	SELECT BasisDate
         , OpeningPointSum
         , CharacterID
         , CharacterName
         , GuildID
         , GuildName
	  FROM #Temp_Result
	 WHERE SortNumber = 1;

	DROP TABLE #Temp_Result;

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
