USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimItems]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimItems](
	[ItemID] [int] NOT NULL,
	[ItemName] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[LevelLimit] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[ItemRankCode] [tinyint] NOT NULL,
	[CashItemFlag] [bit] NOT NULL,
 CONSTRAINT [PK_DimItems] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
