USE [dnworld]
GO
/****** Object:  Table [dbo].[PotentialBoxStat]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PotentialBoxStat](
	[BasisDate] [datetime2](0) NOT NULL,
	[PotentialBoxValue] [int] NOT NULL,
	[UserCount] [int] NULL,
 CONSTRAINT [PK_PotentialBoxStat] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[PotentialBoxValue] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
