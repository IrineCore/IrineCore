USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_SurvivalRate]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_SurvivalRate](
	[MainCategory] [tinyint] NOT NULL,
	[SubCategory] [tinyint] NOT NULL,
	[BasisDate] [date] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[Cnt] [int] NOT NULL,
 CONSTRAINT [PK_ST_SurvivalRate] PRIMARY KEY CLUSTERED 
(
	[MainCategory] ASC,
	[SubCategory] ASC,
	[BasisDate] ASC,
	[JobCode] ASC,
	[CharacterLevel] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
