USE [dnworld]
GO
/****** Object:  Table [dbo].[DNGirlSeasonSchedules]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DNGirlSeasonSchedules](
	[DNGirlSeasonID] [int] IDENTITY(1,1) NOT NULL,
	[SeasonStartDate] [datetime] NOT NULL,
	[SeasonEndDate] [datetime] NOT NULL,
	[CeremonyEndDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DNGirlSeasonSchedules] PRIMARY KEY CLUSTERED 
(
	[DNGirlSeasonID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
