USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_MissionBinary]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE FUNCTION [dbo].[FN_MissionBinary](@binGainMissionBin binary(1250), @intMissionID int)
RETURNS binary(1250)
AS
BEGIN
	RETURN CAST(SUBSTRING(@binGainMissionBin, 1, @intMissionID/8) 
			+ CAST( CAST(SUBSTRING(@binGainMissionBin, @intMissionID/8+1, 1) AS tinyint) | CAST(POWER( 2, 7-@intMissionID%8) AS tinyint) AS VARBINARY)
			+ SUBSTRING(@binGainMissionBin, CAST(@intMissionID/8 AS int)+2, 1250) AS VARBINARY(1250))
END
GO
