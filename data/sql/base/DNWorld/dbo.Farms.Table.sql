USE [dnworld]
GO
/****** Object:  Table [dbo].[Farms]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Farms](
	[FarmID] [int] IDENTITY(1,1) NOT NULL,
	[FarmName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[MapID] [int] NOT NULL,
	[MaxUserCount] [int] NOT NULL,
	[EnableFlag] [bit] NOT NULL,
	[ActivationFlag] [bit] NOT NULL,
	[FarmTypeCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_Farms] PRIMARY KEY CLUSTERED 
(
	[FarmID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
