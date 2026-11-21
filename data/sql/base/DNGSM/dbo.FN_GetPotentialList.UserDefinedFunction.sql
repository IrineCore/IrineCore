USE [dngsm]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_GetPotentialList]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-12-08
description : PotentialID ? ?? ??? ????? ?? (?????)
**/

CREATE FUNCTION [dbo].[FN_GetPotentialList] (
	@PotentialID nvarchar(255)
)
RETURNS nvarchar(200)
AS
BEGIN
	DECLARE @results NVARCHAR(200);	
	
	SELECT @results = COALESCE(@results + ',', '') + _PotentialNo 
	FROM dbo.ExtPotential WITH (NOLOCK)
	WHERE _PotentialID = @PotentialID AND _PotentialNo > 0
	ORDER BY _PotentialNo;

	RETURN @results;
END

GO
