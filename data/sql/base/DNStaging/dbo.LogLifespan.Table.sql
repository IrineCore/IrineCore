USE [dnstaging]
GO
/****** Object:  Table [dbo].[LogLifespan]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[LogLifespan](
	[LogTableName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Lifespan] [int] NOT NULL,
 CONSTRAINT [PK_LogLifespan] PRIMARY KEY CLUSTERED 
(
	[LogTableName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[LogLifespan]  WITH CHECK ADD  CONSTRAINT [CK_LogLifespan_Lifespan] CHECK  (([Lifespan]>=(30)))
GO
ALTER TABLE [dbo].[LogLifespan] CHECK CONSTRAINT [CK_LogLifespan_Lifespan]
GO
