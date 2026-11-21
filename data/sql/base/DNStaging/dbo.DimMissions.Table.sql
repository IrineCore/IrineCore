USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimMissions]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimMissions](
	[MissionID] [int] NOT NULL,
	[MissionName] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[MissionScore] [int] NOT NULL,
 CONSTRAINT [PK_DimMissions] PRIMARY KEY CLUSTERED 
(
	[MissionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
